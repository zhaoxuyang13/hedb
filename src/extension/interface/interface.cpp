#include <interface.hpp>
#include <tee_interface.hpp>
#include <sync.h>
#include <timer.hpp>
#include <stdlib.h> // at exit
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>
#include <unordered_map>
#include <extension.hpp>
#include <sys/mman.h>
#include <fcntl.h>

#define MAX_LOG_SIZE 50000


/* this load barrier is only for arm */
#ifdef __aarch64__
	#define LOAD_BARRIER asm volatile("dsb ld" ::: "memory")
	#define STORE_BARRIER asm volatile("dsb st" ::: "memory")
#elif __x86_64
	#define LOAD_BARRIER ;
	#define STORE_BARRIER ;
#endif

TEEInvoker *TEEInvoker::invoker = NULL;

#define RR_MINIMUM false



#define MAX_NAME_LENGTH 100
#define MAX_PARALLEL_WORKER_SIZE 16    //TODO: the database can only see one buffer allocated to it.
#define MAX_RECORDS_NUM (MAX_PARALLEL_WORKER_SIZE + 1)

bool recordMode = true;
bool replayMode = false;
bool sequence_replay = true;
int records_cnt = 0;
char record_name_prefix[MAX_NAME_LENGTH];
char record_names[MAX_RECORDS_NUM][MAX_NAME_LENGTH];

uint8_t log_buffer[MAX_LOG_SIZE] ;
uint64_t current_log_size = 0;

static inline uint64_t
arm64_pmccntr(void)
{
//    uint64_t tsc;
//    asm volatile("mrs %0, pmccntr_el0" : "=r"(tsc));
//    return tsc;
}

// FILE *write_ptr = 0;
// void flush_to_log_file(uint8_t *buffer, uint64_t size){
//     if (write_ptr == 0)
//     {
//         write_ptr = fopen("test.binlog","wb"); 
//     }
//     fwrite(buffer,size,1,write_ptr);
// }
FILE *write_file_ptr = 0;
void update_write_file_ptr(){
    pid_t pid = getpid();
    char filename[120];
    sprintf(filename, "%s-%d.log", record_name_prefix, pid);
    print_info("%s\n", filename);

    write_file_ptr = fopen(filename,"w+b"); 
}

void flush_write_file_ptr(){
    print_info("fork happens\n");
    fflush(write_file_ptr);
}

FILE *get_write_file_ptr(){
    if (write_file_ptr == 0)
    {
        update_write_file_ptr();
        pthread_atfork(NULL, NULL, update_write_file_ptr);
    }
    return write_file_ptr;
}
void close_write_file_ptr(){
    if(write_file_ptr)
        fclose(write_file_ptr);
}

#define DATA_LENGTH (16 * 1024 * 1024UL)
static unsigned long file_length = 0;
static unsigned long file_cursor = 0;
static char *write_addr = nullptr;
static int write_fd = 0;

void open_write_file() {
    if (!write_fd) {
        pid_t pid = getpid();
        char filename[120];
        sprintf(filename, "%s-%d.log", record_name_prefix, pid);
        print_info("%s\n", filename);

        write_fd = open(filename, O_RDWR | O_CREAT, 0666);
    }
}

char *get_write_buffer(unsigned long length) {
    if (file_cursor + length > file_length) {
        // munmap(write_addr, file_length);
        file_length += DATA_LENGTH;
        ftruncate(write_fd, file_length);
        write_addr = (char *)mmap(NULL, file_length, PROT_READ|PROT_WRITE, MAP_SHARED, write_fd, 0);
        madvise(write_addr + file_cursor, DATA_LENGTH, MADV_SEQUENTIAL);
    }
    char *start = write_addr + file_cursor;
    file_cursor += length;
    return start;
}

FILE *read_file_ptr = 0;
FILE *get_read_file_ptr(int id){
    if (read_file_ptr == 0)
    { 
        char filename[120];
        sprintf(filename, "%s-%d.log", record_name_prefix, id); 
        read_file_ptr = fopen(filename,"r+b"); 
    }
    return read_file_ptr;
}

void close_read_file_ptr(){
    fclose(read_file_ptr);
    read_file_ptr = 0;
}

// TODO: not implemented yet
std::unordered_map<string, string> log_map;
void read_log()
{
    for(int i = 0; i < records_cnt; i ++){
        read_file_ptr = fopen(record_names[i],"r+b");

        fclose(read_file_ptr);
    }
}


/* ----- */

#define BASE64_PAD '='
#define BASE64DE_FIRST '+'
#define BASE64DE_LAST 'z'
#define BASE64_ENCODE_OUT_SIZE(s) ((unsigned int)((((s) + 2) / 3) * 4 + 1))
#define BASE64_DECODE_OUT_SIZE(s) ((unsigned int)(((s) / 4) * 3))

/* BASE 64 encode table */
static const char base64en[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/',
};

/* ASCII order for BASE 64 decode, 255 in unused character */
static const unsigned char base64de[] = {
	/* nul, soh, stx, etx, eot, enq, ack, bel, */
	   255, 255, 255, 255, 255, 255, 255, 255,

	/*  bs,  ht,  nl,  vt,  np,  cr,  so,  si, */
	   255, 255, 255, 255, 255, 255, 255, 255,

	/* dle, dc1, dc2, dc3, dc4, nak, syn, etb, */
	   255, 255, 255, 255, 255, 255, 255, 255,

	/* can,  em, sub, esc,  fs,  gs,  rs,  us, */
	   255, 255, 255, 255, 255, 255, 255, 255,

	/*  sp, '!', '"', '#', '$', '%', '&', ''', */
	   255, 255, 255, 255, 255, 255, 255, 255,

	/* '(', ')', '*', '+', ',', '-', '.', '/', */
	   255, 255, 255,  62, 255, 255, 255,  63,

	/* '0', '1', '2', '3', '4', '5', '6', '7', */
	    52,  53,  54,  55,  56,  57,  58,  59,

	/* '8', '9', ':', ';', '<', '=', '>', '?', */
	    60,  61, 255, 255, 255, 255, 255, 255,

	/* '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', */
	   255,   0,   1,  2,   3,   4,   5,    6,

	/* 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', */
	     7,   8,   9,  10,  11,  12,  13,  14,

	/* 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', */
	    15,  16,  17,  18,  19,  20,  21,  22,

	/* 'X', 'Y', 'Z', '[', '\', ']', '^', '_', */
	    23,  24,  25, 255, 255, 255, 255, 255,

	/* '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', */
	   255,  26,  27,  28,  29,  30,  31,  32,

	/* 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', */
	    33,  34,  35,  36,  37,  38,  39,  40,

	/* 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', */
	    41,  42,  43,  44,  45,  46,  47,  48,

	/* 'x', 'y', 'z', '{', '|', '}', '~', del, */
	    49,  50,  51, 255, 255, 255, 255, 255
};

unsigned int
base64_encode(const unsigned char *in, unsigned int inlen, char *out)
{
	int s;
	unsigned int i;
	unsigned int j;
	unsigned char c;
	unsigned char l;

	s = 0;
	l = 0;
	for (i = j = 0; i < inlen; i++) {
		c = in[i];

		switch (s) {
		case 0:
			s = 1;
			out[j++] = base64en[(c >> 2) & 0x3F];
			break;
		case 1:
			s = 2;
			out[j++] = base64en[((l & 0x3) << 4) | ((c >> 4) & 0xF)];
			break;
		case 2:
			s = 0;
			out[j++] = base64en[((l & 0xF) << 2) | ((c >> 6) & 0x3)];
			out[j++] = base64en[c & 0x3F];
			break;
		}
		l = c;
	}

	switch (s) {
	case 1:
		out[j++] = base64en[(l & 0x3) << 4];
		out[j++] = BASE64_PAD;
		out[j++] = BASE64_PAD;
		break;
	case 2:
		out[j++] = base64en[(l & 0xF) << 2];
		out[j++] = BASE64_PAD;
		break;
	}

	out[j] = 0;

	return j;
}

unsigned int
base64_decode(const char *in, unsigned int inlen, unsigned char *out)
{
	unsigned int i;
	unsigned int j;
	unsigned char c;

	if (inlen & 0x3) {
		return 0;
	}

	for (i = j = 0; i < inlen; i++) {
		if (in[i] == BASE64_PAD) {
			break;
		}
		if (in[i] < BASE64DE_FIRST || in[i] > BASE64DE_LAST) {
			return 0;
		}

		c = base64de[(unsigned char)in[i]];
		if (c == 255) {
			return 0;
		}

		switch (i & 0x3) {
		case 0:
			out[j] = (c << 2) & 0xFF;
			break;
		case 1:
			out[j++] |= (c >> 4) & 0x3;
			out[j] = (c & 0xF) << 4; 
			break;
		case 2:
			out[j++] |= (c >> 2) & 0xF;
			out[j] = (c & 0x3) << 6;
			break;
		case 3:
			out[j++] |= c;
			break;
		}
	}

	return j;
}




/* ----- */
void _print_hex(const char *what, const void *v, const unsigned long l)
{
    char tmp[500];
	const unsigned char *p = (const unsigned char *)v;
	unsigned long x, y = 0, z;
	sprintf(tmp,"%s contents: \n", what);
	for (x = 0; x < l;)
	{
		sprintf(tmp + strlen(tmp),"%02x ", p[x]);
		if (!(++x % 16) || x == l)
		{
			if ((x % 16) != 0)
			{
				z = 16 - (x % 16);
				if (z >= 8)
					sprintf(tmp+ strlen(tmp)," ");
				for (; z != 0; --z)
				{
					sprintf(tmp+ strlen(tmp),"   ");
				}
			}
			sprintf(tmp+ strlen(tmp)," | ");
			for (; y < x; y++)
			{
				if ((y % 8) == 0)
					sprintf(tmp+ strlen(tmp)," ");
				if (isgraph(p[y]))
					sprintf(tmp+ strlen(tmp),"%c", p[y]);
				else
					sprintf(tmp+ strlen(tmp),".");
			}
			sprintf(tmp+ strlen(tmp),"\n");
		}
		else if ((x % 8) == 0)
		{
			sprintf(tmp+ strlen(tmp)," ");
		}
	}
    print_info("%s\n", tmp);
}

void print_base64(const void *cipher, int length, int base64_length, char* message) 
{
    char base64_str[base64_length];
    base64_encode((const unsigned char *)cipher, length, base64_str);
    print_info("%s length: %d, contents: %s", message, length, base64_str);
}

#define ENC_FLOAT4_LENGTH_B64 BASE64_ENCODE_OUT_SIZE(ENC_FLOAT4_LENGTH)
#define ENC_INT32_LENGTH_B64 BASE64_ENCODE_OUT_SIZE(ENC_INT32_LENGTH)
#define ENC_STRING_LENGTH_B64 BASE64_ENCODE_OUT_SIZE(ENC_STRING_LENGTH)
#define ENC_TIMESTAMP_LENGTH_B64 BASE64_ENCODE_OUT_SIZE(ENC_TIMESTAMP_LENGTH)
TEEInvoker::~TEEInvoker() {
    freeBuffer(req_buffer);
}
#define RETRY_FAILED 10086

int blindly_replay_request(void *req_buffer, FILE *read_file_ptr){
    BaseRequest *req_control = static_cast<BaseRequest *>(req_buffer);
    int reqType = req_control->reqType;
    if(reqType>= 101 
    && reqType<= 110){
        char b64_res[ENC_FLOAT4_LENGTH_B64];
        if(reqType == CMD_FLOAT_CMP){
            EncFloatCmpRequestData *req = (EncFloatCmpRequestData *) req_buffer; 
            fscanf(read_file_ptr, "%d\n", &req->cmp);
        }else if(reqType == CMD_FLOAT_SUM_BULK){
            EncFloatBulkRequestData *req = (EncFloatBulkRequestData *) req_buffer; 
            fscanf(read_file_ptr, "%s\n", (char*) &b64_res);
            base64_decode((const char*)b64_res, strlen(b64_res),(unsigned char *)&req->res);
        }else {
            EncFloatCalcRequestData *req = (EncFloatCalcRequestData *) req_buffer;
            fscanf(read_file_ptr, "%s\n", (char*) &b64_res);
            base64_decode((const char*)b64_res, strlen(b64_res),(unsigned char *)&req->res);
        }
    }else if(reqType >= 1
        && reqType <= 10
    ){
        char b64_res[ENC_INT32_LENGTH_B64];
        if(reqType == CMD_INT_CMP){
            EncIntCmpRequestData *req = (EncIntCmpRequestData *) req_buffer; 
            fscanf(read_file_ptr, "%d\n", &req->cmp);
        }else if(reqType == CMD_INT_SUM_BULK){
            EncIntBulkRequestData *req = (EncIntBulkRequestData *) req_buffer;
            fscanf(read_file_ptr, "%s\n",b64_res);
            base64_decode((const  char *)&b64_res, strlen(b64_res), (unsigned char *)&req->res);
        }
        else {
            EncIntCalcRequestData *req = (EncIntCalcRequestData *) req_buffer;
            fscanf(read_file_ptr, "%s\n",b64_res); 
            base64_decode((const char*) b64_res, strlen(b64_res), (unsigned char *)&req->res);
        } 
    }else if(reqType >= 201
        && reqType <= 206
        ){
            char b64_res[ENC_STRING_LENGTH_B64];
            if(reqType == CMD_STRING_CMP ||
                reqType == CMD_STRING_LIKE){
                EncStrCmpRequestData *req = (EncStrCmpRequestData *) req_buffer;
                fscanf(read_file_ptr,"%d\n",&req->cmp);    
            }
            else if(req_control->reqType == CMD_STRING_SUBSTRING){
                SubstringRequestData *req = (SubstringRequestData *) req_buffer;
                fscanf(read_file_ptr,"%s\n",b64_res); 
                base64_decode((const char *)&b64_res, strlen(b64_res), (unsigned char *)&req->res);
            }else {
                EncStrCalcRequestData *req = (EncStrCalcRequestData *) req_buffer;
                fscanf(read_file_ptr,"%s\n",b64_res); 
                base64_decode((const char *)b64_res, strlen(b64_res), (unsigned char *)&req->res);
            } 
    } 
    return 0;
}

int random_replay(void *req_buffer)
{

}

static int previous_op = 0;
int seq_replay(void *req_buffer, FILE *read_file_ptr)
{
    BaseRequest *req_control = static_cast<BaseRequest *>(req_buffer);
    int reqType = req_control->reqType;
    int op, resp;
    uint64_t timestamp;
    fread(&op, sizeof(int), 1, read_file_ptr);
    if (op != reqType) {
        print_error("replay fail at %ld, op: %d, reqType: %d, previous_op: %d", ftell(read_file_ptr), op, reqType, previous_op);
        return -RETRY_FAILED;
    }

    if(reqType>= 101 
    && reqType<= 110){
        EncFloat left,right,res;
        int cmp;
        if(reqType == CMD_FLOAT_CMP){
            EncFloatCmpRequestData *req = (EncFloatCmpRequestData *) req_buffer; 
            char read_buffer[sizeof(int) * 2 + ENC_FLOAT4_LENGTH * 2 + sizeof(uint64_t)];
            char *src = read_buffer;
            fread(read_buffer, sizeof(char), sizeof(int) * 2 + ENC_FLOAT4_LENGTH * 2 + sizeof(uint64_t), read_file_ptr);
            memcpy(&left, src, ENC_FLOAT4_LENGTH);
            src += ENC_FLOAT4_LENGTH;
            memcpy(&right, src, ENC_FLOAT4_LENGTH);
            src += ENC_FLOAT4_LENGTH;
            memcpy(&cmp, src, sizeof(int));
            src += sizeof(int);
            memcpy(&resp, src, sizeof(int));
            src += sizeof(int);
            memcpy(&timestamp, src, sizeof(uint64_t));
            
            if (memcmp(&req->left, &left, ENC_FLOAT4_LENGTH) || memcmp(&req->right, &right, ENC_FLOAT4_LENGTH)) {
                print_error("float cmp fail at %ld", ftell(read_file_ptr));
                return -RETRY_FAILED;
            }
            req->cmp = cmp;
        }else if(reqType == CMD_FLOAT_SUM_BULK){
            EncFloatBulkRequestData *req = (EncFloatBulkRequestData *) req_buffer;
            int bulk_size;
            char read_buffer[sizeof(int) * 2 + ENC_FLOAT4_LENGTH + sizeof(uint64_t)];
            char *src = read_buffer;
            fread(read_buffer, sizeof(char), sizeof(int) * 2 + ENC_FLOAT4_LENGTH + sizeof(uint64_t), read_file_ptr);
            memcpy(&bulk_size, src, sizeof(int));
            src += sizeof(int);
            memcpy(&res, src, ENC_FLOAT4_LENGTH);
            src += ENC_FLOAT4_LENGTH;
            memcpy(&resp, src, sizeof(int));
            src += sizeof(int);
            memcpy(&timestamp, src, sizeof(uint64_t));

            if (bulk_size != req->bulk_size) {
                print_error("float sum fail at %ld, req->bulk_size: %d, bulk_size:%d", ftell(read_file_ptr), req->bulk_size, bulk_size);
                return -RETRY_FAILED;
            }

            char read_buffer_operators[bulk_size * ENC_FLOAT4_LENGTH];
            EncFloat operand;
            src = read_buffer_operators;
            fread(read_buffer_operators, sizeof(char), bulk_size * ENC_FLOAT4_LENGTH, read_file_ptr);
            for (int i = 0; i < bulk_size; i++) {
                memcpy(&operand, src, ENC_FLOAT4_LENGTH);
                if (memcmp(&operand, &req->items[i], ENC_FLOAT4_LENGTH)) {
                    print_error("float sum operands %d different", i);
                    return -RETRY_FAILED;
                }
                src += ENC_FLOAT4_LENGTH;
            }

            memcpy(&req->res, &res, ENC_FLOAT4_LENGTH);
        }
        else {
            EncFloatCalcRequestData *req = (EncFloatCalcRequestData *) req_buffer;
            char read_buffer[sizeof(int) + ENC_FLOAT4_LENGTH * 3 + sizeof(uint64_t)];
            char *src = read_buffer;
            fread(read_buffer, sizeof(char), sizeof(int) + ENC_FLOAT4_LENGTH * 3 + sizeof(uint64_t), read_file_ptr);
            memcpy(&left, src, ENC_FLOAT4_LENGTH);
            src += ENC_FLOAT4_LENGTH;
            memcpy(&right, src, ENC_FLOAT4_LENGTH);
            src += ENC_FLOAT4_LENGTH;
            memcpy(&res, src, ENC_FLOAT4_LENGTH);
            src += ENC_FLOAT4_LENGTH;
            memcpy(&resp, src, sizeof(int));
            src += sizeof(int);
            memcpy(&timestamp, src, sizeof(uint64_t));

            if (memcmp(&left, &req->left, ENC_FLOAT4_LENGTH) || memcmp(&right, &req->right, ENC_FLOAT4_LENGTH)) {
                print_error("float ops fail at %ld", ftell(read_file_ptr));
                return -RETRY_FAILED;
            }
            
            memcpy(&req->res, &res, ENC_FLOAT4_LENGTH);
        }
    }else if(reqType >= 1
        && reqType <= 10
    ){
        EncInt left,right,res;
        int cmp;
        if(reqType == CMD_INT_CMP){
            EncIntCmpRequestData *req = (EncIntCmpRequestData *) req_buffer; 
            char read_buffer[sizeof(int) * 2 + ENC_INT32_LENGTH * 2 + sizeof(uint64_t)];
            char *src = read_buffer;
            fread(read_buffer, sizeof(char), sizeof(int) * 2 + ENC_INT32_LENGTH * 2 + sizeof(uint64_t), read_file_ptr);
            memcpy(&left, src, ENC_INT32_LENGTH);
            src += ENC_INT32_LENGTH;
            memcpy(&right, src, ENC_INT32_LENGTH);
            src += ENC_INT32_LENGTH;
            memcpy(&cmp, src, sizeof(int));
            src += sizeof(int);
            memcpy(&resp, src, sizeof(int));
            src += sizeof(int);
            memcpy(&timestamp, src, sizeof(uint64_t));

            if (memcmp(&left, &req->left, ENC_INT32_LENGTH) || memcmp(&right, &req->right, ENC_INT32_OPE_LENGTH)) {
                print_error("int cmp fail at %ld", ftell(read_file_ptr));
                return -RETRY_FAILED;
            }

            req->cmp = cmp;
        }else if(reqType == CMD_INT_SUM_BULK){
            EncIntBulkRequestData *req = (EncIntBulkRequestData *) req_buffer;
            int bulk_size;
            char read_buffer[sizeof(int) * 2 + ENC_INT32_LENGTH + sizeof(uint64_t)];
            char *src = read_buffer;
            fread(read_buffer, sizeof(char), sizeof(int) * 2 + ENC_INT32_LENGTH + sizeof(uint64_t), read_file_ptr);
            memcpy(&bulk_size, src, sizeof(int));
            src += sizeof(int);
            memcpy(&res, src, ENC_INT32_LENGTH);
            src += ENC_INT32_LENGTH;
            memcpy(&resp, src, sizeof(int));
            src += sizeof(int);
            memcpy(&timestamp, src, sizeof(uint64_t));

            if (bulk_size != req->bulk_size) {
                print_error("int sum fail at %ld, req->bulk_size: %d, bulk_size:%d", ftell(read_file_ptr), req->bulk_size, bulk_size);
                return -RETRY_FAILED;
            }

            char read_buffer_operators[bulk_size * ENC_INT32_LENGTH];
            EncFloat operand;
            src = read_buffer_operators;
            fread(read_buffer_operators, sizeof(char), bulk_size * ENC_INT32_LENGTH, read_file_ptr);
            for (int i = 0; i < bulk_size; i++) {
                memcpy(&operand, src, ENC_INT32_LENGTH);
                if (memcmp(&operand, &req->items[i], ENC_INT32_LENGTH)) {
                    print_error("int sum operands %d different", i);
                    return -RETRY_FAILED;
                }
                src += ENC_INT32_LENGTH;
            }

            memcpy(&req->res, &res, ENC_INT32_LENGTH);
        }
        else {
            EncIntCalcRequestData *req = (EncIntCalcRequestData *) req_buffer;
            char read_buffer[sizeof(int) + ENC_INT32_LENGTH * 3 + sizeof(uint64_t)];
            char *src = read_buffer;
            fread(read_buffer, sizeof(char), sizeof(int) + ENC_INT32_LENGTH * 3 + sizeof(uint64_t), read_file_ptr);
            memcpy(&left, src, ENC_INT32_LENGTH);
            src += ENC_INT32_LENGTH;
            memcpy(&right, src, ENC_INT32_LENGTH);
            src += ENC_INT32_LENGTH;
            memcpy(&res, src, ENC_INT32_LENGTH);
            src += ENC_INT32_LENGTH;
            memcpy(&resp, src, sizeof(int));
            src += sizeof(int);
            memcpy(&timestamp, src, sizeof(uint64_t));

            if (memcmp(&left, &req->left, ENC_INT32_LENGTH) || memcmp(&right, &req->right, ENC_INT32_LENGTH)) {
                print_error("int ops fail at %ld", ftell(read_file_ptr));
                return -RETRY_FAILED;
            }
            
            memcpy(&req->res, &res, ENC_INT32_LENGTH);
        }
    } else if (reqType >= 201 && reqType <= 206)
    {
        EncStr left, right, res;
        int cmp;
        if (reqType == CMD_STRING_CMP ||
            reqType == CMD_STRING_LIKE) {
            EncStrCmpRequestData *req = (EncStrCmpRequestData *)req_buffer;
            int left_length, right_length;
            fread(&left_length, sizeof(int), 1, read_file_ptr);
            fread(&right_length, sizeof(int), 1, read_file_ptr);

            if (left_length != encstr_size(req->left) || right_length != encstr_size(req->right)) {
                print_error("string cmp fail at %ld, length not right", ftell(read_file_ptr));
                return -RETRY_FAILED;
            }

            char read_buffer[left_length + right_length + sizeof(int) * 2 + sizeof(uint64_t)];
            char *src = read_buffer;
            fread(read_buffer, sizeof(char), left_length + right_length + sizeof(int) * 2 + sizeof(uint64_t), read_file_ptr);
            memcpy(&left, src, left_length);
            src += left_length;
            memcpy(&right, src, right_length);
            src += right_length;
            memcpy(&cmp, src, sizeof(int));
            src += sizeof(int);
            memcpy(&resp, src, sizeof(int));
            src += sizeof(int);
            memcpy(&timestamp, src, sizeof(uint64_t));

            // if (memcmp(&left, &req->left, left_length) || memcmp(&right, &req->right, right_length)) {
            //     print_error("string cmp/like(%d) fail at %ld, contents mismatch", op, ftell(read_file_ptr));
            //     return -RETRY_FAILED;
            // }
            req->cmp = cmp;
        }
        else if (req_control->reqType == CMD_STRING_SUBSTRING) {
            SubstringRequestData *req = (SubstringRequestData *)req_buffer;
            EncStr str;
            EncInt begin, end;
            int str_length, result_length;
            fread(&str_length, sizeof(int), 1, read_file_ptr);
            fread(&result_length, sizeof(int), 1, read_file_ptr);

            if (str_length != encstr_size(req->str)) {
                print_error("string substring fail at %ld, length not right", ftell(read_file_ptr));
                return -RETRY_FAILED;
            }

            char read_buffer[str_length + result_length + sizeof(int) + ENC_INT32_LENGTH * 2 + sizeof(uint64_t)];
            char *src = read_buffer;
            fread(read_buffer, sizeof(char), str_length + result_length + sizeof(int) + ENC_INT32_LENGTH * 2 + sizeof(uint64_t), read_file_ptr);
            memcpy(&str, src, str_length);
            src += str_length;
            memcpy(&begin, src, ENC_INT32_LENGTH);
            src += ENC_INT32_LENGTH;
            memcpy(&end, src, ENC_INT32_LENGTH);
            src += ENC_INT32_LENGTH;
            memcpy(&res, src, result_length);
            src += result_length;
            memcpy(&resp, src, sizeof(int));
            src += sizeof(int);
            memcpy(&timestamp, src, sizeof(uint64_t));

            if (memcmp(&str, &req->str, str_length) || memcmp(&begin, &req->begin, ENC_INT32_LENGTH) || memcmp(&end, &req->end, ENC_INT32_LENGTH))
            {
                print_error("string substring fail at %ld, contents mismatch", ftell(read_file_ptr));
                return -RETRY_FAILED;
            }
            memcpy(&req->res, &res, result_length);
        }
        else {
            EncStrCalcRequestData *req = (EncStrCalcRequestData *)req_buffer;
            int left_length, right_length, res_length;
            fread(&left_length, sizeof(int), 1, read_file_ptr);
            fread(&right_length, sizeof(int), 1, read_file_ptr);
            fread(&res_length, sizeof(int), 1, read_file_ptr);


            if (left_length != encstr_size(req->left) || right_length != encstr_size(req->right)) {
                print_error("string ops fail at %ld, length not right", ftell(read_file_ptr));
                return -RETRY_FAILED;
            }

            char read_buffer[sizeof(int) + left_length + right_length + res_length + sizeof(uint64_t)];
            fread(read_buffer, sizeof(char), sizeof(int) + left_length + right_length + res_length + sizeof(uint64_t), read_file_ptr);
            char *src = read_buffer;
            memcpy(&left, src, left_length);
            src += left_length;
            memcpy(&right, src, right_length);
            src += right_length;
            memcpy(&res, src, res_length);
            src += res_length;
            memcpy(&resp, src, sizeof(int));
            src += sizeof(int);
            memcpy(&timestamp, src, sizeof(uint64_t));
            
            if (memcmp(&left, &req->left, left_length) || memcmp(&right, &req->right, right_length)) {
                print_error("string ops fail at %ld, contents mismatch", ftell(read_file_ptr));
                return -RETRY_FAILED;
            }
            
            memcpy(&req->res, &res, res_length);
        }
    } else if (reqType >= 150 && reqType <= 153) {
        if (req_control->reqType == CMD_TIMESTAMP_CMP) {
            int cmp;
            EncTimestamp left, right;
            EncTimestampCmpRequestData *req = (EncTimestampCmpRequestData *)req_buffer;
            char read_buffer[sizeof(int) * 2 + ENC_TIMESTAMP_LENGTH * 2 + sizeof(uint64_t)];
            fread(read_buffer, sizeof(char), sizeof(int) * 2 + ENC_TIMESTAMP_LENGTH * 2 + sizeof(uint64_t), read_file_ptr);
            char *src = read_buffer;
            memcpy(&left, src, ENC_TIMESTAMP_LENGTH);
            src += ENC_TIMESTAMP_LENGTH;
            memcpy(&right, src, ENC_TIMESTAMP_LENGTH);
            src += ENC_TIMESTAMP_LENGTH;
            memcpy(&cmp, src, sizeof(int));
            src += sizeof(int);
            memcpy(&resp, src, sizeof(int));
            src += sizeof(int);
            memcpy(&timestamp, src, sizeof(uint64_t));

            if (memcmp(&left, &req->left, ENC_TIMESTAMP_LENGTH) || memcmp(&right, &req->right, ENC_TIMESTAMP_LENGTH)) {
                print_info("timestamp cmp fail at %ld", ftell(read_file_ptr));
                return -RETRY_FAILED;
            }
            req->cmp = cmp;
        } else if (req_control->reqType == CMD_TIMESTAMP_EXTRACT_YEAR) {
            EncTimestamp in;
            EncInt out;
            EncTimestampExtractYearRequestData *req = (EncTimestampExtractYearRequestData *) req_buffer;
            char read_buffer[sizeof(int) + ENC_TIMESTAMP_LENGTH + ENC_INT32_LENGTH + sizeof(uint64_t)];
            fread(read_buffer, sizeof(char), sizeof(int) + ENC_TIMESTAMP_LENGTH + ENC_INT32_LENGTH + sizeof(uint64_t), read_file_ptr);
            char *src = read_buffer;
            memcpy(&in, src, ENC_TIMESTAMP_LENGTH);
            src += ENC_TIMESTAMP_LENGTH;
            memcpy(&out, src, ENC_INT32_LENGTH);
            src += ENC_INT32_LENGTH;
            memcpy(&resp, src, sizeof(int));
            src += sizeof(int);
            memcpy(&timestamp, src, sizeof(uint64_t));

            if (memcmp(&in, &req->in, ENC_TIMESTAMP_LENGTH)) {
                print_info("timestamp extract fail at %ld", ftell(read_file_ptr));
                return -RETRY_FAILED;
            }
            memcpy(&req->res, &out, ENC_INT32_LENGTH);
        }
    }
    previous_op = op;
    return resp;
}

int try_replay_request(void *req_buffer, FILE *read_file_ptr){
    if (sequence_replay) {
        return seq_replay(req_buffer, read_file_ptr);
    } else {
        return random_replay(req_buffer);
    }
}

int replay_request(void *req){
    static FILE *read_file_ptr = nullptr; 
    if(read_file_ptr == nullptr){
        for(int i = 0; i < records_cnt; i ++){
            read_file_ptr = fopen(record_names[i],"r+b");
            int ret = try_replay_request(req, read_file_ptr);
            if(ret == -RETRY_FAILED){
                fclose(read_file_ptr);
                read_file_ptr = nullptr;
            }else {
                return ret;
            }
        }
        BaseRequest *req_control = static_cast<BaseRequest *>(req); 
        print_info("no valid record file found. error %d\n", req_control->reqType);
        return -1; // not valid record file found. error.
    }else {
        #if RR_MINIMUM
            return blindly_replay_request(req, read_file_ptr);
        #else 
            return try_replay_request(req, read_file_ptr);
        #endif
    }
}
void record_request_plaintext(void *req_buffer, FILE *write_file_ptr){
    BaseRequest *req_control = static_cast<BaseRequest *>(req_buffer);  
    if(req_control->reqType != CMD_FLOAT_ENC 
    && req_control->reqType != CMD_FLOAT_DEC
    && req_control->reqType >= 101
    && req_control->reqType <= 109
    ){
        if(req_control->reqType == CMD_FLOAT_CMP){
            EncFloatCmpRequestData *req = (EncFloatCmpRequestData *) req_buffer;
            float left = *((float*)&req->left);
            float right = *((float*)&req->right);
            fprintf(write_file_ptr,"%d %f %f %d %d\n",
                req_control->reqType, 
                left,
                right,
                req->cmp,
                req_control->resp);
        }else if(req_control->reqType == CMD_FLOAT_SUM_BULK){
            EncFloatBulkRequestData *req = (EncFloatBulkRequestData *) req_buffer;
            float res = *((float*)&req->res);
            fprintf(write_file_ptr,"%d %d %f %d",
                req_control->reqType, 
                req->bulk_size,
                res,
                req_control->resp);
            for(int i = 0; i < req->bulk_size; i++){
                float tmp =*((float*)&req->items[i]);
                fprintf(write_file_ptr," %f", tmp);
            }
            fprintf(write_file_ptr, "\n");
            
        }
        else {
            EncFloatCalcRequestData *req = (EncFloatCalcRequestData *) req_buffer;
            float left = *((float*)&req->left);
            float right =*((float*)&req->right);
            float res =*((float*)&req->res);
            fprintf(write_file_ptr,"%d %f %f %f %d\n",
                req->op, 
                left,
                right,
                res,
                req_control->resp);
        }
        // flush_to_log_file((uint8_t *)ch, req_size);
    } // end of float
    else if(req_control->reqType >= 1
    && req_control->reqType <= 10
    && req_control->reqType != CMD_INT_ENC 
    && req_control->reqType != CMD_INT_DEC
    ){
        if(req_control->reqType == CMD_INT_CMP){
            EncIntCmpRequestData *req = (EncIntCmpRequestData *) req_buffer;
            int left = *((int*)&req->left);
            int right =*((int*)&req->right);
            fprintf(write_file_ptr,"%d %d %d %d %d\n",
                req_control->reqType, 
                left,
                right,
                req->cmp,
                req_control->resp);
        }else if(req_control->reqType == CMD_INT_SUM_BULK){
            EncIntBulkRequestData *req = (EncIntBulkRequestData *) req_buffer;
            int res =*((int*)&req->res);
            fprintf(write_file_ptr,"%d %d %d %d",
                req_control->reqType, 
                req->bulk_size,
                res,
                req_control->resp);
            for(int i = 0; i < req->bulk_size; i++){
                int tmp =*((int* )&req->items[i]);
                fprintf(write_file_ptr," %d", tmp);
            }
            fprintf(write_file_ptr, "\n");
        }
        else {
            EncIntCalcRequestData *req = (EncIntCalcRequestData *) req_buffer;
            int left = *((int*)&req->left);
            int right =*((int*)&req->right);
            int res = *((int*)&req->res);
            fprintf(write_file_ptr,"%d %d %d %d %d\n",
                req->op, 
                left,
                right,
                res,
                req_control->resp);
        } 
    }else if(req_control->reqType != CMD_STRING_ENC 
    && req_control->reqType != CMD_STRING_DEC
    && req_control->reqType >= 201
    && req_control->reqType <= 206
    ){
        if(req_control->reqType == CMD_STRING_CMP ||
            req_control->reqType == CMD_STRING_LIKE){
            EncStrCmpRequestData *req = (EncStrCmpRequestData *) req_buffer;
            char *left = (char *) &req->left.enc_cstr;
            left[req->left.len - IV_SIZE - TAG_SIZE ] = '\0';
            char *right = (char *) &req->right.enc_cstr;
            right[req->right.len - IV_SIZE - TAG_SIZE ] = '\0';
            fprintf(write_file_ptr,"%d %s %s %d %d\n",
                req_control->reqType, 
                left,
                right,
                req->cmp,
                req_control->resp);

        }
        else if(req_control->reqType == CMD_STRING_SUBSTRING){
            SubstringRequestData *req = (SubstringRequestData *) req_buffer;
            char *str = (char *) &req->str.enc_cstr;
            str[req->str.len - IV_SIZE - TAG_SIZE ] = '\0';
            char *res = (char *) &req->res.enc_cstr;
            res[req->res.len - IV_SIZE - TAG_SIZE] = '\0';
            int begin = *((int*)&req->begin);
            int end =*((int*)&req->end);
            fprintf(write_file_ptr,"%d %s %d %d %s %d\n",
                req_control->reqType, 
                str,
                begin,
                end,
                res,
                req_control->resp); 
        }else {
            EncStrCalcRequestData *req = (EncStrCalcRequestData *) req_buffer;
            char *left = (char *) &req->left.enc_cstr;
            left[req->left.len - IV_SIZE - TAG_SIZE] = '\0';
            char *right = (char *) &req->right.enc_cstr;
            right[req->right.len - IV_SIZE - TAG_SIZE] = '\0';
            char *res = (char *) &req->res.enc_cstr;
            res[req->res.len - IV_SIZE - TAG_SIZE] = '\0';
            fprintf(write_file_ptr,"%d %s %s %s %d\n",
                req->op, 
                left,
                right,
                res,
                req_control->resp);
        } 
    }
}

void record_request_full(void *req_buffer){
    BaseRequest *req_control = static_cast<BaseRequest *>(req_buffer);  
    if(req_control->reqType != CMD_FLOAT_ENC 
    && req_control->reqType != CMD_FLOAT_DEC
    && req_control->reqType >= 101
    && req_control->reqType <= 109
    ){
        if(req_control->reqType == CMD_FLOAT_CMP){
            EncFloatCmpRequestData *req = (EncFloatCmpRequestData *) req_buffer;
            size_t length = sizeof(int) * 3 + ENC_FLOAT4_LENGTH * 2 + sizeof(uint64_t);
            char *dst = get_write_buffer(length);
            memcpy(dst, &req_control->reqType, sizeof(int));
            dst += sizeof(int);
            memcpy(dst, &req->left, ENC_FLOAT4_LENGTH);
            dst += ENC_FLOAT4_LENGTH;
            memcpy(dst, &req->right, ENC_FLOAT4_LENGTH);
            dst += ENC_FLOAT4_LENGTH;
            memcpy(dst, &req->cmp, sizeof(int));
            dst += sizeof(int);
            memcpy(dst, &req_control->resp, sizeof(int));
            dst += sizeof(int);
            uint64_t timestamp = arm64_pmccntr();
            memcpy(dst, &timestamp, sizeof(uint64_t));
        }else if(req_control->reqType == CMD_FLOAT_SUM_BULK){
            EncFloatBulkRequestData *req = (EncFloatBulkRequestData *) req_buffer;
            size_t length = sizeof(int) * 3 + ENC_FLOAT4_LENGTH + sizeof(uint64_t);
            char *dst = get_write_buffer(length);
            memcpy(dst, &req_control->reqType, sizeof(int));
            dst += sizeof(int);
            memcpy(dst, &req->bulk_size, sizeof(int));
            dst += sizeof(int);
            memcpy(dst, &req->res, ENC_FLOAT4_LENGTH);
            dst += ENC_FLOAT4_LENGTH;
            memcpy(dst, &req_control->resp, sizeof(int));
            dst += sizeof(int);
            uint64_t timestamp = arm64_pmccntr();
            memcpy(dst, &timestamp, sizeof(uint64_t));
            
            dst = get_write_buffer(req->bulk_size * ENC_FLOAT4_LENGTH);
            for(int i = 0; i < req->bulk_size; i++){
                memcpy(dst, &req->items[i], ENC_FLOAT4_LENGTH);
                dst += ENC_FLOAT4_LENGTH;
            }
        }
        else {
            EncFloatCalcRequestData *req = (EncFloatCalcRequestData *) req_buffer;
            size_t length = sizeof(int) * 2 + ENC_FLOAT4_LENGTH * 3 + sizeof(uint64_t);
            char *dst = get_write_buffer(length);
            memcpy(dst, &req->op, sizeof(int));
            dst += sizeof(int);
            memcpy(dst, &req->left, ENC_FLOAT4_LENGTH);
            dst += ENC_FLOAT4_LENGTH;
            memcpy(dst, &req->right, ENC_FLOAT4_LENGTH);
            dst += ENC_FLOAT4_LENGTH;
            memcpy(dst, &req->res, ENC_FLOAT4_LENGTH);
            dst += ENC_FLOAT4_LENGTH;
            memcpy(dst, &req_control->resp, sizeof(int));
            dst += sizeof(int);
            uint64_t timestamp = arm64_pmccntr();
            memcpy(dst, &timestamp, sizeof(uint64_t));
        }

    } // end of float
    else if(req_control->reqType >= 1
    && req_control->reqType <= 10
    && req_control->reqType != CMD_INT_ENC 
    && req_control->reqType != CMD_INT_DEC
    ){
        if(req_control->reqType == CMD_INT_CMP){
            EncIntCmpRequestData *req = (EncIntCmpRequestData *) req_buffer;
            size_t length = sizeof(int) * 3 + ENC_INT32_LENGTH * 2 + sizeof(uint64_t);
            char *dst = get_write_buffer(length);
            memcpy(dst, &req_control->reqType, sizeof(int));
            dst += sizeof(int);
            memcpy(dst, &req->left, ENC_INT32_LENGTH);
            dst += ENC_INT32_LENGTH;
            memcpy(dst, &req->right, ENC_INT32_LENGTH);
            dst += ENC_INT32_LENGTH;
            memcpy(dst, &req->cmp, sizeof(int));
            dst += sizeof(int);
            memcpy(dst, &req_control->resp, sizeof(int));
            dst += sizeof(int);
            uint64_t timestamp = arm64_pmccntr();
            memcpy(dst, &timestamp, sizeof(uint64_t));
        } else if(req_control->reqType == CMD_INT_SUM_BULK){
            EncIntBulkRequestData *req = (EncIntBulkRequestData *) req_buffer;
            size_t length = sizeof(int) * 3 + ENC_INT32_LENGTH + sizeof(uint64_t);
            char *dst = get_write_buffer(length);
            memcpy(dst, &req_control->reqType, sizeof(int));
            dst += sizeof(int);
            memcpy(dst, &req->bulk_size, sizeof(int));
            dst += sizeof(int);
            memcpy(dst, &req->res, ENC_INT32_LENGTH);
            dst += ENC_INT32_LENGTH;
            memcpy(dst, &req_control->resp, sizeof(int));
            dst += sizeof(int);
            uint64_t timestamp = arm64_pmccntr();
            memcpy(dst, &timestamp, sizeof(uint64_t));

            dst = get_write_buffer(req->bulk_size * ENC_INT32_LENGTH);
            for(int i = 0; i < req->bulk_size; i++){
                memcpy(dst, &req->items[i], ENC_INT32_LENGTH);
                dst += ENC_INT32_LENGTH;
            }
        }
        else {
            EncIntCalcRequestData *req = (EncIntCalcRequestData *) req_buffer;
            size_t length = sizeof(int) * 2 + ENC_INT32_LENGTH * 3 + sizeof(uint64_t);
            char *dst = get_write_buffer(length);
            memcpy(dst, &req->op, sizeof(int));
            dst += sizeof(int);
            memcpy(dst, &req->left, ENC_INT32_LENGTH);
            dst += ENC_INT32_LENGTH;
            memcpy(dst, &req->right, ENC_INT32_LENGTH);
            dst += ENC_INT32_LENGTH;
            memcpy(dst, &req->res, ENC_INT32_LENGTH);
            dst += ENC_INT32_LENGTH;
            memcpy(dst, &req_control->resp, sizeof(int));
            dst += sizeof(int);
            uint64_t timestamp = arm64_pmccntr();
            memcpy(dst, &timestamp, sizeof(uint64_t));
        } 
    }else if(req_control->reqType != CMD_STRING_ENC 
    && req_control->reqType != CMD_STRING_DEC
    && req_control->reqType >= 201
    && req_control->reqType <= 206
    ){
        if(req_control->reqType == CMD_STRING_CMP ||
            req_control->reqType == CMD_STRING_LIKE){
            EncStrCmpRequestData *req = (EncStrCmpRequestData *) req_buffer;
            int left_length = encstr_size(req->left), right_length = encstr_size(req->right);
            int length = sizeof(int) * 5 + left_length + right_length + sizeof(uint64_t);
            char *dst = get_write_buffer(length);
            memcpy(dst, &req_control->reqType, sizeof(int));
            dst += sizeof(int);
            memcpy(dst, &left_length, sizeof(int));
            dst += sizeof(int);
            memcpy(dst, &right_length, sizeof(int));
            dst += sizeof(int);
            memcpy(dst, &req->left, left_length);
            dst += left_length;
            memcpy(dst, &req->right, right_length);
            dst += right_length;
            memcpy(dst, &req->cmp, sizeof(int));
            dst += sizeof(int);
            memcpy(dst, &req_control->resp, sizeof(int));
            dst += sizeof(int);
            uint64_t timestamp = arm64_pmccntr();
            memcpy(dst, &timestamp, sizeof(uint64_t));
        }
        else if(req_control->reqType == CMD_STRING_SUBSTRING){
            SubstringRequestData *req = (SubstringRequestData *) req_buffer;
            int str_length = encstr_size(req->str), result_length = encstr_size(req->res);
            size_t length = sizeof(int) * 4 + str_length + result_length + 2 * ENC_INT32_LENGTH + sizeof(uint64_t);
            char *dst = get_write_buffer(length);
            memcpy(dst, &req_control->reqType, sizeof(int));
            dst += sizeof(int);
            memcpy(dst, &str_length, sizeof(int));
            dst += sizeof(int);
            memcpy(dst, &result_length, sizeof(int));
            dst += sizeof(int);
            memcpy(dst, &req->str, str_length);
            dst += str_length;
            memcpy(dst, &req->begin, ENC_INT32_LENGTH);
            dst += ENC_INT32_LENGTH;
            memcpy(dst, &req->end, ENC_INT32_LENGTH);
            dst += ENC_INT32_LENGTH;
            memcpy(dst, &req->res, result_length);
            dst += result_length;
            memcpy(dst, &req_control->resp, sizeof(int));
            dst += sizeof(int);
            uint64_t timestamp = arm64_pmccntr();
            memcpy(dst, &timestamp, sizeof(uint64_t));
        }else {
            EncStrCalcRequestData *req = (EncStrCalcRequestData *) req_buffer;
            int left_length = encstr_size(req->left), right_length = encstr_size(req->right), res_length = encstr_size(req->res);
            int length = sizeof(int) * 5 + left_length + right_length + res_length + sizeof(uint64_t);
            char *dst = get_write_buffer(length);
            memcpy(dst, &req->op, sizeof(int));
            dst += sizeof(int);
            memcpy(dst, &left_length, sizeof(int));
            dst += sizeof(int);
            memcpy(dst, &right_length, sizeof(int));
            dst += sizeof(int);
            memcpy(dst, &res_length, sizeof(int));
            dst += sizeof(int);
            memcpy(dst, &req->left, left_length);
            dst += left_length;
            memcpy(dst, &req->right, right_length);
            dst += right_length;
            memcpy(dst, &req->res, res_length);
            dst += res_length;
            memcpy(dst, &req_control->resp, sizeof(int));
            dst += sizeof(int);
            uint64_t timestamp = arm64_pmccntr();
            memcpy(dst, &timestamp, sizeof(uint64_t));
        } 
    } else if (req_control->reqType >= 150
    && req_control->reqType <= 153
    && req_control->reqType != CMD_TIMESTAMP_DEC
    && req_control->reqType != CMD_TIMESTAMP_ENC
    ) {
        if (req_control->reqType == CMD_TIMESTAMP_CMP) {
            EncTimestampCmpRequestData *req = (EncTimestampCmpRequestData *) req_buffer;
            int length = sizeof(int) * 3 + ENC_TIMESTAMP_LENGTH * 2 + sizeof(uint64_t);
            char *dst = get_write_buffer(length);
            memcpy(dst, &req_control->reqType, sizeof(int));
            dst += sizeof(int);
            memcpy(dst, &req->left, ENC_TIMESTAMP_LENGTH);
            dst += ENC_TIMESTAMP_LENGTH;
            memcpy(dst, &req->right, ENC_TIMESTAMP_LENGTH);
            dst += ENC_TIMESTAMP_LENGTH;
            memcpy(dst, &req->cmp, sizeof(int));
            dst += sizeof(int);
            memcpy(dst, &req_control->resp, sizeof(int));
            dst += sizeof(int);
            uint64_t timestamp = arm64_pmccntr();
            memcpy(dst, &timestamp, sizeof(uint64_t));
        } else if (req_control->reqType == CMD_TIMESTAMP_EXTRACT_YEAR) {
            EncTimestampExtractYearRequestData *req = (EncTimestampExtractYearRequestData *) req_buffer;
            size_t length = sizeof(int) * 2 + ENC_TIMESTAMP_LENGTH + ENC_INT32_LENGTH + sizeof(uint64_t);
            char *dst = get_write_buffer(length);
            memcpy(dst, &req_control->reqType, sizeof(int));
            dst += sizeof(int);
            memcpy(dst, &req->in, ENC_TIMESTAMP_LENGTH);
            dst += ENC_TIMESTAMP_LENGTH;
            memcpy(dst, &req->res, ENC_INT32_LENGTH);
            dst += ENC_INT32_LENGTH;
            memcpy(dst, &req_control->resp, sizeof(int));
            dst += sizeof(int);
            uint64_t timestamp = arm64_pmccntr();
            memcpy(dst, &timestamp, sizeof(uint64_t));
        }
    }
}
void record_request_res(void *req_buffer, FILE *write_file_ptr){
    BaseRequest *req_control = static_cast<BaseRequest *>(req_buffer);  
    if(req_control->reqType >= 101
    && req_control->reqType <= 109
    ){
        char b64_res[ENC_FLOAT4_LENGTH_B64];
        
        if(req_control->reqType == CMD_FLOAT_CMP){
            EncFloatCmpRequestData *req = (EncFloatCmpRequestData *) req_buffer;
            fprintf(write_file_ptr,"%d\n", req->cmp);
        }else if(req_control->reqType == CMD_FLOAT_SUM_BULK){
            EncFloatBulkRequestData *req = (EncFloatBulkRequestData *) req_buffer;
            base64_encode((const unsigned char *)&req->res, ENC_FLOAT4_LENGTH, b64_res);
            fprintf(write_file_ptr,"%s\n", b64_res);
        }
        else {
            EncFloatCalcRequestData *req = (EncFloatCalcRequestData *) req_buffer;
            base64_encode((const unsigned char *)&req->res, ENC_FLOAT4_LENGTH, b64_res);

            fprintf(write_file_ptr,"%s\n",b64_res);
        }
    } // end of float
    else if(req_control->reqType >= 1
    && req_control->reqType <= 10
    ){
        char b64_res[ENC_INT32_LENGTH_B64];
        if(req_control->reqType == CMD_INT_CMP){
            EncIntCmpRequestData *req = (EncIntCmpRequestData *) req_buffer;
            fprintf(write_file_ptr,"%d\n", req->cmp);
        }else if(req_control->reqType == CMD_INT_SUM_BULK){
            EncIntBulkRequestData *req = (EncIntBulkRequestData *) req_buffer;
            base64_encode((const unsigned char *)&req->res, ENC_INT32_LENGTH, b64_res);
            fprintf(write_file_ptr,"%s\n", b64_res);
        }
        else {
            EncIntCalcRequestData *req = (EncIntCalcRequestData *) req_buffer;
            base64_encode((const unsigned char *)&req->res, ENC_INT32_LENGTH, b64_res);
            fprintf(write_file_ptr,"%s\n", b64_res);
        } 
    }else if(req_control->reqType >= 201
    && req_control->reqType <= 206
    ){
        char b64_res[ENC_STRING_LENGTH_B64];
        if(req_control->reqType == CMD_STRING_CMP ||
            req_control->reqType == CMD_STRING_LIKE){
            EncStrCmpRequestData *req = (EncStrCmpRequestData *) req_buffer;
            fprintf(write_file_ptr,"%d\n", req->cmp);
        }
        else if(req_control->reqType == CMD_STRING_SUBSTRING){
            SubstringRequestData *req = (SubstringRequestData *) req_buffer;
            base64_encode((const unsigned char *)&req->res, encstr_size(req->res), b64_res);
            fprintf(write_file_ptr,"%s\n", b64_res); 
        }else {
            EncStrCalcRequestData *req = (EncStrCalcRequestData *) req_buffer;
            base64_encode((const unsigned char *)&req->res, encstr_size(req->res), b64_res);
            fprintf(write_file_ptr,"%s\n", b64_res);
        } 
    }
}

void record_request(void *req){
    static int first_record = true ;
    if(first_record){
        first_record = false;
        open_write_file();
        record_request_full(req);
    }else {
        #if RR_MINIMUM
            record_request_res(req, write_file_ptr);
        #else 
            record_request_full(req);
        #endif
    }
}

char null_buffer[1024*1024];
int TEEInvoker::sendRequest(Request *req) {
    int resp;
    req->serializeTo(req_buffer);

    // print_info("done seriale\n");
    BaseRequest *req_control = static_cast<BaseRequest *>(req_buffer);
    /* TODO write barrier */
    // print_info("REQUIEST sent");

    if(replayMode){
        if((req_control->reqType >= 101
        && req_control->reqType <= 109
        && req_control->reqType != CMD_FLOAT_ENC 
        && req_control->reqType != CMD_FLOAT_DEC
        )||(req_control->reqType >= 1
        && req_control->reqType <= 10
        && req_control->reqType != CMD_INT_ENC 
        && req_control->reqType != CMD_INT_DEC
        ) ||(req_control->reqType >= 201
        && req_control->reqType <= 206
        && req_control->reqType != CMD_STRING_ENC 
        && req_control->reqType != CMD_STRING_DEC
        ) || (req_control->reqType >= 150
        && req_control->reqType <= 153
        && req_control->reqType != CMD_TIMESTAMP_DEC
        && req_control->reqType != CMD_TIMESTAMP_ENC
        )){
            /* replay_request will answer request written to req_buffer */
            int resp = replay_request(req_buffer);

            /* then copy result from req_buffer to destination buffer */
            req->copyResultFrom(req_buffer);
            return resp;
        }
    }
    // static myTime time;
	// static double timer = 0,duration = 0;
	// time.tic();
    STORE_BARRIER;
    req_control->status = SENT;
    /* wait for status */
    while (req_control->status != DONE)
        YIELD_PROCESSOR;
    /* TODO read barrier */
    LOAD_BARRIER;
    req->copyResultFrom(req_buffer);
    resp = req_control->resp;
    // print_info("req type: %d, get response from udf", req_control->reqType);

    // if(req_control->reqType == CMD_INT_CMP){
    //     // print_info("TEST LATENCY\n");
    //     for(int i = 1000000; i >= 0; i --){
    //         req->copyResultFrom(req_buffer);
    //     }
    // }

    // char ch[1000];
    
    /* log */
    if(recordMode){
        if((req_control->reqType >= 101
        && req_control->reqType <= 109
        && req_control->reqType != CMD_FLOAT_ENC 
        && req_control->reqType != CMD_FLOAT_DEC
        )||(req_control->reqType >= 1
        && req_control->reqType <= 10
        && req_control->reqType != CMD_INT_ENC 
        && req_control->reqType != CMD_INT_DEC
        ) ||(req_control->reqType >= 201
        && req_control->reqType <= 206
        && req_control->reqType != CMD_STRING_ENC 
        && req_control->reqType != CMD_STRING_DEC
        )||(req_control->reqType >= 150
        && req_control->reqType <= 153
        && req_control->reqType != CMD_TIMESTAMP_DEC
        && req_control->reqType != CMD_TIMESTAMP_ENC
        )){
            record_request(req_buffer);
        }
    }

    /* read-write barrier, no read move after this barrier, no write move before this barrier */
    
    req_control->status = NONE;

    // time.toc();
    // duration = time.getDuration() / 1000000;
    // timer += duration;
    // if(req_control->reqType == CMD_FLOAT_DEC || req_control->reqType == CMD_INT_DEC){
    //     print_info("total duration in ms: sum %f ", timer );
    // }

    return resp;
}

extern FILE *plain_file;
void exit_handler(){
    print_info("EXIT handler called\n");
    TEEInvoker *invoker = TEEInvoker::getInstance();
    delete invoker;
    if (write_fd) {
        close(write_fd);
    }
    // if(write_ptr != 0)
    //     fclose(write_ptr);

    // if(plain_file != 0)
    //    fclose(plain_file); 
}


void enter_replay_mode(){
    if(recordMode){
        recordMode = false;
        close_write_file_ptr();
    }
    replayMode = true;

    // char* s = PG_GETARG_CSTRING(0);
    // strncpy(record_name_prefix, s, strlen(s));
    // strcat(record_name_prefix, "-");
    // print_info("%s\n", s);

    // DIR *dir;
    // struct dirent *ent;
    // if ((dir = opendir ("/usr/local/pgsql/data")) != NULL) {
    //     // print_info("open directory success\n");
    //     /* print all the files and directories within directory */
    //     while ((ent = readdir(dir)) != NULL) {
    //         if(0 == strncmp(record_name_prefix, ent->d_name, strlen(record_name_prefix))){ // if prefix match add to names list
    //             strncpy(record_names[records_cnt], ent->d_name, strlen(ent->d_name));
    //             records_cnt ++;
    //         }
    //     }
    //     char tmp[256];
    //     sprintf(tmp, "find %d log files\n", records_cnt);
    //     for(int i = 0; i < records_cnt; i ++){
    //         sprintf(tmp + strlen(tmp), "%d: %s\n", i, record_names[i]);
    //     }
    //     print_info("%s\n",tmp);
    //     closedir (dir);
    // } else {
    //     /* could not open directory */
    //     print_info("could not open directory\n");
    //     return ;
    // }

}


TEEInvoker::TEEInvoker() {
    // print_info("get shared buffer");
    req_buffer = getSharedBuffer(sizeof (EncIntBulkRequestData));
    if(req_buffer == 0){
        enter_replay_mode();
        req_buffer = getMockBuffer(sizeof (EncIntBulkRequestData));
        /* mock buffer is connected with replayer */
    }
    BaseRequest *req_control = static_cast<BaseRequest *>(req_buffer);
    req_control->status = NONE;
    // print_info("buffer got");
    atexit(exit_handler);
} //
