#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "crypto.h"
#include "enc_ops.h"
#include "sync.h"
#include "sim.hpp"
#include "request_types.h"
#include <pthread.h>

/* this load barrier is only for arm */
#ifdef __aarch64__
	#define LOAD_BARRIER asm volatile("dsb ld" ::: "memory")
	#define STORE_BARRIER asm volatile("dsb st" ::: "memory")
#elif __x86_64
	#define LOAD_BARRIER ;
	#define STORE_BARRIER ;
#endif


uint8_t *decrypt_dst;
size_t decrypt_src_len, decrypt_dst_len;
volatile int decrypt_status = DONE;

uint8_t decrypt_buffer[sizeof(EncCStr) * 2];
uint8_t plain_buffer[2048];
void *decrypt_thread(void * arg){
	while (1)
	{
		if(decrypt_status == SENT){
			LOAD_BARRIER;
			int resp = decrypt_bytes(decrypt_buffer, decrypt_src_len, plain_buffer, decrypt_dst_len);
			STORE_BARRIER;
			decrypt_status = DONE;
		}else if(decrypt_status == EXIT){
			break;
		}else {
			;
		}
	}
	return 0;
}

int decrypt_bytes_para(uint8_t *pSrc, size_t src_len, uint8_t *pDst, size_t exp_dst_len)
{	
	assert(decrypt_status == DONE);

	static int inited = false;
	if(!inited){
		pid_t pid;
		pthread_t thread;
		pthread_create(&thread, nullptr, decrypt_thread, nullptr);
		inited = true;
	}
	memcpy(decrypt_buffer, pSrc, src_len);
	// decrypt_src = pSrc;
	decrypt_src_len = src_len;
	// decrypt_dst = pDst;
	decrypt_dst_len = exp_dst_len;
	STORE_BARRIER;
	decrypt_status = SENT;
	return 0;
}

void decrypt_wait(uint8_t *pDst, size_t exp_dst_len){
	while (decrypt_status != DONE)
	{
		YIELD_PROCESSOR;
	}
	LOAD_BARRIER;
	memcpy(pDst, plain_buffer, exp_dst_len);
}


int decrypt_bytes(uint8_t *pSrc, size_t src_len, uint8_t *pDst, size_t exp_dst_len)
{	
	// _print_hex("dec ", pSrc, src_len);

	size_t dst_len = 0;
	int resp = 0;

	// uint64_t duration =0,timer = 0;
	// uint64_t start,end;
	// uint32_t freq = read_cntfrq();
	// start = read_cntpct();

	// for (size_t i = 0; i < 1000; i++)
	// {
	// uint8_t *iv_pos = pSrc;
	// uint8_t *tag_pos = pSrc+IV_SIZE;
	// uint8_t *data_pos = pSrc+IV_SIZE+TAG_SIZE;
	// _print_hex("dec--enc-iv: ", (void *)iv_pos, IV_SIZE);
	// _print_hex("dec--enc-tag: ", (void *)tag_pos, TAG_SIZE);
	// _print_hex("dec--enc-txt: ", (void *)data_pos, exp_dst_len);
	if(src_len <= IV_SIZE + TAG_SIZE ){
		printf("error src len");
		return -1;
	}
	resp = gcm_decrypt(pSrc, src_len, pDst, &dst_len);
	// printf("after dec");
	assert(dst_len == exp_dst_len);

	// }

	// end= read_cntpct();
	// duration = (end - start) * 1000000 / freq;
	// timer += duration;
	// printf("1000 dec duration in us %d",duration);


	// memset(pDst, 0, dst_len);
	// memcpy(pDst, pSrc, dst_len);
	if (resp !=0)
	{
		static __thread pid_t tid = gettid();
		printf("tid %d, resp %d\n", tid,  resp);
		_print_hex("dec from ", pSrc, src_len);
		_print_hex("dec to ", pDst, dst_len);
	}
	
	return resp;
}

/* Encrypts byte array by aesgcm mode
 @input: uint8_t array - pointer to a byte array
         size_t - length of the array
         uint8_t array - pointer to result array
         size_t - length of result array (IV_SIZE + length of array +
 TAG_SIZE)
 @return:
    * SGX_error, if there was an error during encryption/decryption
    0, otherwise
*/
int encrypt_bytes(uint8_t *pSrc, size_t src_len, uint8_t *pDst, size_t exp_dst_len)
{

	size_t dst_len = exp_dst_len;
	int resp = 0;

	resp = gcm_encrypt(pSrc, src_len, pDst, &dst_len);
	// printf("after enc");
	assert(dst_len == exp_dst_len);
	// {
	// 	_print_hex("enc from ", pSrc, src_len);
	// 	_print_hex("enc to ", pDst, dst_len);
	// }
	
	return resp;
}

FILE *write_ptr = 0;
// #define printf(...) fprintf(write_ptr, __VA_ARGS__ )

int shmid;
void *shmaddr = NULL;
void exit_handler(){
    // 4. detach shared memory
    if (shmaddr != 0 && shmdt(shmaddr) == -1) {
        perror("shmdt failed\n");
    }
 
    // 5. delete shared memory
    if (shmaddr != 0 && shmid != 0 && shmctl(shmid, IPC_RMID, 0) == -1) {
        perror("shmctl delete shared memory failed\n");
    }
	
	if(write_ptr != 0){
		fclose(write_ptr);
	}
}


int main(int argc,char *argv[]){
    
	key_t key = (key_t) 2333;
	if(argc == 2)
		key = (key_t) atoi(argv[1]);
	else if(argc > 2) {
		printf("error argc\n");
		exit(-1);
	}

    int data_size = sizeof(EncIntBulkRequestData);
	char filename[64];
	sprintf(filename, "pid-%d.log", key);
	write_ptr = fopen(filename, "w+"); 
	
	atexit(exit_handler);

    // 1. create shared memory
    shmid = shmget(key, data_size, 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget failed\n");
        exit(EXIT_FAILURE);
    }
 
    // 2. attach shared memory
    shmaddr = shmat(shmid, NULL, 0);
    if (shmaddr == (void *)-1) {
        perror("shmat failed\n");
        exit(EXIT_FAILURE);
    }
 
    // 3. read data to shared memory
	int counter = 0;
	BaseRequest *req = (BaseRequest *)shmaddr;
	while(1){
		if (req->status == EXIT)
		{
			decrypt_status = EXIT;
			printf("SIM-TA Exit %d\n", counter);
			break;
		}
		else if(req->status == SENT)
		{
			LOAD_BARRIER;

			counter ++;
			if (counter % 100000 == 0)
				printf("counter %d\n", counter++);

			handle_ops(req);

			if (req->resp != 0)
			{
				printf("TA error %d, %d\n",req->resp,counter);
			}

			STORE_BARRIER;
			req->status = DONE;
		}
		else 
			YIELD_PROCESSOR;
	}

 
    return 0;
}
