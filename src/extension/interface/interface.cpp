#include <interface.hpp>
#include <tee_interface.hpp>
#include <sync.h>
#include <timer.hpp>
#include <stdlib.h> // at exit
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>
#include <extension.hpp>

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

bool recordMode = false;
bool replayMode = false;
int records_cnt = 0;
char record_name_prefix[MAX_NAME_LENGTH];
char record_names[MAX_RECORDS_NUM][MAX_NAME_LENGTH];

uint8_t log_buffer[MAX_LOG_SIZE] ;
uint64_t current_log_size = 0;

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

    write_file_ptr = fopen(filename,"w+"); 
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


FILE *read_file_ptr = 0;
FILE *get_read_file_ptr(int id){
    if (read_file_ptr == 0)
    { 
        char filename[120];
        sprintf(filename, "%s-%d.log", record_name_prefix, id); 
        read_file_ptr = fopen(filename,"r+"); 
    }
    return read_file_ptr;
}

void close_read_file_ptr(){
    fclose(read_file_ptr);
    read_file_ptr = 0;
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

#define ENC_FLOAT4_LENGTH_B64 BASE64_ENCODE_OUT_SIZE(ENC_FLOAT4_LENGTH)
#define ENC_INT32_LENGTH_B64 BASE64_ENCODE_OUT_SIZE(ENC_INT32_LENGTH)
#define ENC_STRING_LENGTH_B64 BASE64_ENCODE_OUT_SIZE(ENC_STRING_LENGTH)
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
int try_replay_request(void *req_buffer, FILE *read_file_ptr){
    BaseRequest *req_control = static_cast<BaseRequest *>(req_buffer);
    int reqType = req_control->reqType;
    int op, resp;
    if(reqType>= 101 
    && reqType<= 110){

        char b64_left[ENC_FLOAT4_LENGTH_B64],b64_right[ENC_FLOAT4_LENGTH_B64],b64_res[ENC_FLOAT4_LENGTH_B64];
        EncFloat left,right,res;
        int cmp;
        if(reqType == CMD_FLOAT_CMP){
            EncFloatCmpRequestData *req = (EncFloatCmpRequestData *) req_buffer; 
            fscanf(read_file_ptr, "%d %s %s %d %d\n", &op, 
            b64_left, b64_right, &cmp, &resp);
            base64_decode((const char*) b64_left, strlen(b64_left), (unsigned char *)&left); 
            base64_decode((const char*) b64_right, strlen(b64_right), (unsigned char *)&right);
            if(op != reqType 
                || memcmp(&left, &req->left, ENC_FLOAT4_LENGTH)
                || memcmp(&right, &req->right, ENC_FLOAT4_LENGTH)
                )
                return -RETRY_FAILED;
            req->cmp = cmp;
        }else if(reqType == CMD_FLOAT_SUM_BULK){
            EncFloatBulkRequestData *req = (EncFloatBulkRequestData *) req_buffer;
            int bulk_size;
            fscanf(read_file_ptr, "%d %d %s %d\n", &op, 
            &bulk_size, b64_res, &resp);
            if(op == reqType || bulk_size != req->bulk_size){
                return -RETRY_FAILED;
            }
            base64_decode((const  char *)&b64_res, strlen(b64_res), (unsigned char *)&req->res);
        }
        else {
            EncFloatCalcRequestData *req = (EncFloatCalcRequestData *) req_buffer;
            fscanf(read_file_ptr, "%d %s %s %s %d\n", &op, 
            b64_left, b64_right, b64_res, &resp); 
            base64_decode((const char*) b64_left, strlen(b64_left), (unsigned char *)&left); 
            base64_decode((const char*) b64_right, strlen(b64_right), (unsigned char *)&right);
            base64_decode((const char*) b64_res, strlen(b64_res), (unsigned char *)&res);
            if(op != reqType 
                || memcmp(&left, &req->left, ENC_FLOAT4_LENGTH)
                || memcmp(&right, &req->right, ENC_FLOAT4_LENGTH)
                ) {
                print_info("not correct record file %d, %d\n", op, reqType);
                return -RETRY_FAILED;
            }
            req->res = res;
        }
    }else if(reqType >= 1
        && reqType <= 10
    ){
        int op = 0;
        char b64_left[ENC_INT32_LENGTH_B64],b64_right[ENC_INT32_LENGTH_B64],b64_res[ENC_INT32_LENGTH_B64];
        EncInt left,right,res;
        int cmp, resp;
        if(reqType == CMD_INT_CMP){
            EncIntCmpRequestData *req = (EncIntCmpRequestData *) req_buffer; 
            fscanf(read_file_ptr, "%d %s %s %d %d\n", &op, 
            b64_left, b64_right, &cmp, &resp);
            base64_decode((const char*) b64_left, strlen(b64_left), (unsigned char *)&left); 
            base64_decode((const char*) b64_right, strlen(b64_right), (unsigned char *)&right);
            if(op != reqType 
                || memcmp(&left, &req->left, ENC_FLOAT4_LENGTH)
                || memcmp(&right, &req->right, ENC_FLOAT4_LENGTH)
                )
                return -RETRY_FAILED;
            req->cmp = cmp;
        }else if(reqType == CMD_INT_SUM_BULK){
            EncIntBulkRequestData *req = (EncIntBulkRequestData *) req_buffer;
            int bulk_size;
            fscanf(read_file_ptr, "%d %d %s %d\n", &op, 
            &bulk_size, b64_res, &resp);
            if(op == reqType || bulk_size != req->bulk_size){
                return -RETRY_FAILED;
            }
            base64_decode((const  char *)&b64_res, strlen(b64_res), (unsigned char *)&req->res);
        }
        else {
            EncIntCalcRequestData *req = (EncIntCalcRequestData *) req_buffer;
            fscanf(read_file_ptr, "%d %s %s %s %d\n", &op, 
            b64_left, b64_right, b64_res, &resp); 
            base64_decode((const char*) b64_left, strlen(b64_left), (unsigned char *)&left); 
            base64_decode((const char*) b64_right, strlen(b64_right), (unsigned char *)&right);
            base64_decode((const char*) b64_res, strlen(b64_res), (unsigned char *)&res);
            if(op != reqType 
                || memcmp(&left, &req->left, ENC_INT32_LENGTH)
                || memcmp(&right, &req->right, ENC_INT32_LENGTH)
                ) {
                return -RETRY_FAILED;
            }
            req->res = res;
        } 
    }else if(reqType >= 201
        && reqType <= 206
        ){
            int op = 0;
            char b64_left[ENC_STRING_LENGTH_B64],b64_right[ENC_STRING_LENGTH_B64],b64_res[ENC_STRING_LENGTH_B64];
            EncStr left,right,res;
            int cmp, resp;
            if(reqType == CMD_STRING_CMP ||
                reqType == CMD_STRING_LIKE){
                EncStrCmpRequestData *req = (EncStrCmpRequestData *) req_buffer;
                fscanf(read_file_ptr,"%d %s %s %d %d\n",
                    &op, 
                    b64_left,
                    b64_right,
                    &cmp,
                    &resp);    
                base64_decode((const char *)b64_left, strlen(b64_left), (unsigned char *)&left);
                base64_decode((const char *)b64_right, strlen(b64_right), (unsigned char *)&right);
                if(op != reqType 
                    || memcmp(&left, &req->left, encstr_size(left))
                    || memcmp(&right, &req->right, encstr_size(right))
                    ){
                    print_info("%d, %d, %d, %d, left %s, right %s\n",op, reqType, 
                        memcmp(&left, &req->left, encstr_size(left)),
                        memcmp(&right, &req->right, encstr_size(right)),
                        b64_left, b64_right);
                    print_info("left len %d, origin left len %d\n", left.len, req->left.len);
                    print_info("right len %d, origin right len %d\n", right.len, req->right.len);
                    return -RETRY_FAILED;
                }
                req->cmp = cmp; 
            }
            else if(req_control->reqType == CMD_STRING_SUBSTRING){
                SubstringRequestData *req = (SubstringRequestData *) req_buffer;
                char begin[ENC_INT32_LENGTH_B64], end[ENC_INT32_LENGTH_B64];
                EncInt b, e;
                fscanf(read_file_ptr,"%d %s %s %s %s %d %d\n",
                    &op, 
                    b64_left,
                    begin,
                    end,
                    b64_res,
                    &cmp,
                    &resp); 
                base64_decode((const char *)&b64_left, strlen(b64_left), (unsigned char *)&left);
                base64_decode((const char *)&begin, strlen(begin), (unsigned char *)&b);
                base64_decode((const char *)&end, strlen(end), (unsigned char *)&e);
                base64_decode((const char *)&b64_res, strlen(b64_res), (unsigned char *)&res);
                if(op != reqType 
                    || memcmp(&left, &req->str, encstr_size(req->str))
                    || memcmp(&begin, &req->begin, ENC_INT32_LENGTH)
                    || memcmp(&end, &req->end, ENC_INT32_LENGTH)
                    )
                    return -RETRY_FAILED; 
                req->res = res; 
            }else {
                EncStrCalcRequestData *req = (EncStrCalcRequestData *) req_buffer;
                fscanf(read_file_ptr,"%d %s %s %s %d\n",
                    &op, 
                    b64_left,
                    b64_right,
                    b64_res,
                    &resp); 
                base64_decode((const char *)b64_left, strlen(b64_left), (unsigned char *)&left);
                base64_decode((const char *)b64_right, strlen(b64_right), (unsigned char *)&right); 
                base64_decode((const char *)b64_res, strlen(b64_res), (unsigned char *)&res);
                if(op != reqType 
                    || memcmp(&left, &req->left, encstr_size(req->left))
                    || memcmp(&right, &req->right, encstr_size(req->right))
                    )
                    return -RETRY_FAILED; 
                req->res = res;  
            } 
        }
    return resp;
}

int replay_request(void *req){
    static FILE *read_file_ptr = nullptr; 
    if(read_file_ptr == nullptr){
        for(int i = 0; i < records_cnt; i ++){
            read_file_ptr = fopen(record_names[i],"r+");
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

void record_request_full(void *req_buffer, FILE *write_file_ptr){
    BaseRequest *req_control = static_cast<BaseRequest *>(req_buffer);  
    if(req_control->reqType != CMD_FLOAT_ENC 
    && req_control->reqType != CMD_FLOAT_DEC
    && req_control->reqType >= 101
    && req_control->reqType <= 109
    ){
        char b64_left[ENC_FLOAT4_LENGTH_B64],b64_right[ENC_FLOAT4_LENGTH_B64],b64_res[ENC_FLOAT4_LENGTH_B64];
        memset(b64_left, 0, sizeof(b64_left));
        memset(b64_right, 0, sizeof(b64_right));
        memset(b64_res, 0, sizeof(b64_res));
        if(req_control->reqType == CMD_FLOAT_CMP){
            EncFloatCmpRequestData *req = (EncFloatCmpRequestData *) req_buffer;
            base64_encode((const unsigned char *)&req->left, ENC_FLOAT4_LENGTH, b64_left);
            base64_encode((const unsigned char *)&req->right, ENC_FLOAT4_LENGTH, b64_right);
            fprintf(write_file_ptr,"%d %s %s %d %d\n",
                req_control->reqType, 
                b64_left,
                b64_right,
                req->cmp,
                req_control->resp);
        }else if(req_control->reqType == CMD_FLOAT_SUM_BULK){
            EncFloatBulkRequestData *req = (EncFloatBulkRequestData *) req_buffer;
            base64_encode((const unsigned char *)&req->res, ENC_FLOAT4_LENGTH, b64_res);
            fprintf(write_file_ptr,"%d %d %s %d\n",
                req_control->reqType, 
                req->bulk_size,
                b64_res,
                req_control->resp);
        }
        else {
            EncFloatCalcRequestData *req = (EncFloatCalcRequestData *) req_buffer;
            base64_encode((const unsigned char *)&req->left, ENC_FLOAT4_LENGTH, b64_left);
            base64_encode((const unsigned char *)&req->right, ENC_FLOAT4_LENGTH, b64_right);
            base64_encode((const unsigned char *)&req->res, ENC_FLOAT4_LENGTH, b64_res);

            fprintf(write_file_ptr,"%d %s %s %s %d\n",
                req->op, 
                b64_left,
                b64_right,
                b64_res,
                req_control->resp);
        }
        // flush_to_log_file((uint8_t *)ch, req_size);
    } // end of float
    else if(req_control->reqType >= 1
    && req_control->reqType <= 10
    && req_control->reqType != CMD_INT_ENC 
    && req_control->reqType != CMD_INT_DEC
    ){
        char b64_left[ENC_INT32_LENGTH_B64],b64_right[ENC_INT32_LENGTH_B64],b64_res[ENC_INT32_LENGTH_B64];
        memset(b64_left, 0, sizeof(b64_left));
        memset(b64_right, 0, sizeof(b64_right));
        memset(b64_res, 0, sizeof(b64_res));
        if(req_control->reqType == CMD_INT_CMP){
            EncIntCmpRequestData *req = (EncIntCmpRequestData *) req_buffer;
            base64_encode((const unsigned char *)&req->left, ENC_INT32_LENGTH, b64_left);
            base64_encode((const unsigned char *)&req->right, ENC_INT32_LENGTH, b64_right);
            fprintf(write_file_ptr,"%d %s %s %d %d\n",
                req_control->reqType, 
                b64_left,
                b64_right,
                req->cmp,
                req_control->resp);
        }else if(req_control->reqType == CMD_INT_SUM_BULK){
            EncIntBulkRequestData *req = (EncIntBulkRequestData *) req_buffer;
            base64_encode((const unsigned char *)&req->res, ENC_INT32_LENGTH, b64_res);
            fprintf(write_file_ptr,"%d %d %s %d\n",
                req_control->reqType, 
                req->bulk_size,
                b64_res,
                req_control->resp);
        }
        else {
            EncIntCalcRequestData *req = (EncIntCalcRequestData *) req_buffer;
            base64_encode((const unsigned char *)&req->left, ENC_INT32_LENGTH, b64_left);
            base64_encode((const unsigned char *)&req->right, ENC_INT32_LENGTH, b64_right);
            base64_encode((const unsigned char *)&req->res, ENC_INT32_LENGTH, b64_res);

            fprintf(write_file_ptr,"%d %s %s %s %d\n",
                req->op, 
                b64_left,
                b64_right,
                b64_res,
                req_control->resp);
        } 
    }else if(req_control->reqType != CMD_STRING_ENC 
    && req_control->reqType != CMD_STRING_DEC
    && req_control->reqType >= 201
    && req_control->reqType <= 206
    ){
        char b64_left[ENC_STRING_LENGTH_B64],b64_right[ENC_STRING_LENGTH_B64],b64_res[ENC_STRING_LENGTH_B64];
        memset(b64_left, 0, sizeof(b64_left));
        memset(b64_right, 0, sizeof(b64_right));
        memset(b64_res, 0, sizeof(b64_res));
        if(req_control->reqType == CMD_STRING_CMP ||
            req_control->reqType == CMD_STRING_LIKE){
            EncStrCmpRequestData *req = (EncStrCmpRequestData *) req_buffer;
            base64_encode((const unsigned char *)&req->left, encstr_size(req->left), b64_left);
            base64_encode((const unsigned char *)&req->right, encstr_size(req->right), b64_right);
            fprintf(write_file_ptr,"%d %s %s %d %d\n",
                req_control->reqType, 
                b64_left,
                b64_right,
                req->cmp,
                req_control->resp);
        }
        else if(req_control->reqType == CMD_STRING_SUBSTRING){
            SubstringRequestData *req = (SubstringRequestData *) req_buffer;
            char begin[ENC_INT32_LENGTH_B64], end[ENC_INT32_LENGTH_B64];
            base64_encode((const unsigned char *)&req->str, encstr_size(req->str), b64_left);
            base64_encode((const unsigned char *)&req->begin, ENC_INT32_LENGTH, begin);
            base64_encode((const unsigned char *)&req->end, ENC_INT32_LENGTH, end);
            base64_encode((const unsigned char *)&req->res, encstr_size(req->res), b64_res);
            fprintf(write_file_ptr,"%d %s %s %s %s %d\n",
                req_control->reqType, 
                b64_left,
                begin,
                end,
                b64_res,
                req_control->resp); 
        }else {
            EncStrCalcRequestData *req = (EncStrCalcRequestData *) req_buffer;
            base64_encode((const unsigned char *)&req->left, encstr_size(req->left), b64_left);
            base64_encode((const unsigned char *)&req->right, encstr_size(req->right), b64_right);
            base64_encode((const unsigned char *)&req->res, encstr_size(req->res), b64_res);
            fprintf(write_file_ptr,"%d %s %s %s %d\n",
                req->op, 
                b64_left,
                b64_right,
                b64_res,
                req_control->resp);
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
        record_request_full(req, get_write_file_ptr());
    }else {
        #if RR_MINIMUM
            record_request_res(req, write_file_ptr);
        #else 
            record_request_full(req, get_write_file_ptr());
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
        )){
            int resp = replay_request(req_buffer);
            req->copyResultFrom(req_buffer);
            return resp;
        }
    }
    // static myTime time;
	// static double timer = 0,duration = 0;
	// time.tic();
    STORE_BARRIER;
    req_control->status = SENT;
    // print_info("done sent\n");
    /* wait for status */
    while (req_control->status != DONE)
        YIELD_PROCESSOR;
    /* TODO read barrier */
    LOAD_BARRIER;
    req->copyResultFrom(req_buffer);
    resp = req_control->resp;

    // if(req_control->reqType == CMD_INT_CMP){
    //     // print_info("TEST LATENCY\n");
    //     for(int i = 1000000; i >= 0; i --){
    //         req->copyResultFrom(req_buffer);
    //     }
    // }

    // char ch[1000];
    // print_info("done copy\n");
    
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
    // if(write_ptr != 0)
    //     fclose(write_ptr);

    // if(plain_file != 0)
    //    fclose(plain_file); 
}


TEEInvoker::TEEInvoker() {
    // print_info("get shared buffer");
    req_buffer = getSharedBuffer(sizeof (EncIntBulkRequestData));
    BaseRequest *req_control = static_cast<BaseRequest *>(req_buffer);
    req_control->status = NONE;
    // print_info("buffer got");
    atexit(exit_handler);
} //
