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
#include "ops_server.h"
#include <sys/wait.h>
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
void *posix_shm_addr = NULL;
void posix_shm_exit_handler(){
    // 4. detach shared memory
    if (posix_shm_addr != 0 && shmdt(posix_shm_addr) == -1) {
        perror("shmdt failed\n");
    }
	
	if(write_ptr != 0){
		fclose(write_ptr);
	}
}

void *get_shmem_posix(size_t size){
	// 1. create shared memory
	int key = 666;
	printf("shm size is %lx\n", size);
    shmid = shmget(key, size, 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget failed\n");
        exit(EXIT_FAILURE);
    }
    // 2. attach shared memory
    posix_shm_addr = shmat(shmid, NULL, 0);
    if (posix_shm_addr == (void *)-1) {
        perror("shmat failed\n");
        exit(EXIT_FAILURE);
    }
	atexit(posix_shm_exit_handler);
	return posix_shm_addr;
}

pid_t fork_ops_process(void *shm_addr){
    pid_t pid = fork();
	if(pid != 0){ // father
		return pid;
	}
	// child
	// after fork, child inherit all attached shared memory (man shmat)
	printf("waiting on shm_addr %p\n", shm_addr);
	int counter = 0;
	BaseRequest *req = (BaseRequest *)shm_addr;
	while(1){
		if (req->status == EXIT)
		{
			decrypt_status = EXIT;
			printf("SIM-TA Exit %d\n", counter);
			exit(0);
		}
		else if(req->status == SENT)
		{
			LOAD_BARRIER;

			// counter ++;
			// if (counter % 100000 == 0)
			// 	printf("counter %d\n", counter++);

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
	

	// child should never return.
	exit(0);
}

#define SHM_SIZE (16*1024*1024)
#define META_REQ_SIZE 1024
#define REQ_REGION_SIZE 1024*1024
#define REGION_N_OFFSET(n) (META_REQ_SIZE + REQ_REGION_SIZE * n)
#define MAX_REGION_NUM 16
int main(int argc,char *argv[]){

	int data_size = SHM_SIZE;
	pid_t child_pids[20] = {};

	OpsServer *req = (OpsServer *) get_shmem_posix(data_size);
	memset(req, 0, sizeof(OpsServer));
	while (1)
	{
		if(req->status == SHM_GET){
			// allocate a empty region
			printf("processing get\n");
			for(int i = 0;i < MAX_REGION_NUM; i++){
				if(GET(req->bitmap, i) == 0){
					SET(req->bitmap, i);
					req->ret_id = i;
					void *addr = (void*)req  + REGION_N_OFFSET(i);
					printf("allocate %d, base addr %p, alloc addr %p\n",i,req, addr);
					pid_t child = fork_ops_process(addr);
					printf("child pid %d\n",child);
					child_pids[i] = child;
					break;
				}
			}

			STORE_BARRIER; // store everything before DONE.
			req->status = SHM_DONE;
			printf("get request done\n");
		}else if(req->status == SHM_FREE){
			// free a region
			printf("processing free\n");
			LOAD_BARRIER;
			int id = req->free_id;
			BaseRequest *base_req = (BaseRequest *)((void*)req  + REGION_N_OFFSET(id));
			printf("free %d, %p\n", id, base_req);
			base_req->status = EXIT;
			assert(GET(req->bitmap, id) == 1);
			CLEAR(req->bitmap, id);
			waitpid(child_pids[id], nullptr, 0); // wait until child pid exit;
			printf("waitpid pid %d\n", child_pids[id]);

			STORE_BARRIER; // store everything before DONE.
			req->status = SHM_DONE;
			printf("free request done\n");
		}else {
			YIELD_PROCESSOR;
		}
	}
}
