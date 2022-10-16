#include <interface.hpp>
#include <tee_interface.hpp>
#include <sync.h>
#include <extension_helper.hpp>
#include <timer.hpp>
#include <stdlib.h> // at exit
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#define MAX_LOG_SIZE 50000

TEEInvoker *TEEInvoker::invoker = NULL;


#define MAX_NAME_LENGTH 100
#define MAX_PARALLEL_WORKER_SIZE 16
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
FILE *get_write_file_ptr(){
    if (write_file_ptr == 0)
    {
        pid_t pid = getpid();
        char filename[120];
        sprintf(filename, "%s-%d.log", record_name_prefix, pid);
        write_file_ptr = fopen(filename,"w+"); 
    }
    return write_file_ptr;
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




//----------------------------------------------------
// Using two-byte lookup table
// must call here before calling the above
//----------------------------------------------------
static char Base64Digits[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
unsigned short Base64Digits8192[4096];

WORD* gpLookup16 = 0;
static BYTE LookupDigits[] = { 0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0, // gap: ctrl
                               // chars
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0, // gap: ctrl
                               // chars
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0, // gap: spc,!"#$%'()*
                               62, // +
                               0,
                               0,
                               0, // gap ,-.
                               63, // /
                               52,
                               53,
                               54,
                               55,
                               56,
                               57,
                               58,
                               59,
                               60,
                               61, // 0-9
                               0,
                               0,
                               0, // gap: :;<
                               99, //  = (end padding)
                               0,
                               0,
                               0, // gap: >?@
                               0,
                               1,
                               2,
                               3,
                               4,
                               5,
                               6,
                               7,
                               8,
                               9,
                               10,
                               11,
                               12,
                               13,
                               14,
                               15,
                               16,
                               17,
                               18,
                               19,
                               20,
                               21,
                               22,
                               23,
                               24,
                               25, // A-Z
                               0,
                               0,
                               0,
                               0,
                               0,
                               0, // gap: [\]^_`
                               26,
                               27,
                               28,
                               29,
                               30,
                               31,
                               32,
                               33,
                               34,
                               35,
                               36,
                               37,
                               38,
                               39,
                               40,
                               41,
                               42,
                               43,
                               44,
                               45,
                               46,
                               47,
                               48,
                               49,
                               50,
                               51, // a-z
                               0,
                               0,
                               0,
                               0, // gap: {|}~ (and the rest...)
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0 };

void SetupLookup16()
{
    int nLenTbl = 256 * 256; // yes, the table is 128Kb!
    if (NULL == gpLookup16)
    {
        gpLookup16 = new WORD[nLenTbl];
    }
    WORD* p = gpLookup16;
    for (int j = 0; j < 256; j++)
    {
        for (int k = 0; k < 256; k++)
        {
            WORD w;
            w = LookupDigits[k] << 8;
            w |= LookupDigits[j] << 2; // pre-shifted! See notes
            *p++ = w;
        }
    }
}
static void
SetupTable8192()
{
    int j, k;
    for (j = 0; j < 64; j++)
    {
        for (k = 0; k < 64; k++)
        {
            unsigned short w;
            w = Base64Digits[k] << 8;
            w |= Base64Digits[j];
            Base64Digits8192[(j * 64) + k] = w;
        }
    }
}

int ToBase64Fast(const unsigned char* pSrc, int nLenSrc, char* pDst, int nLenDst)
{
    SetupTable8192();

    int nLenOut = ((nLenSrc + 2) / 3) * 4; // 4 out for every 3 in, rounded up
    if (nLenOut + 1 > nLenDst)
    {
        return (0); // fail!
    }

    unsigned short* pwDst = (unsigned short*)pDst;
    while (nLenSrc > 2)
    {
        unsigned int n = pSrc[0]; // xxx1
        n <<= 8; // xx1x
        n |= pSrc[1]; // xx12
        n <<= 8; // x12x
        n |= pSrc[2]; // x123

        pwDst[0] = Base64Digits8192[n >> 12];
        pwDst[1] = Base64Digits8192[n & 0x00000fff];

        nLenSrc -= 3;
        pwDst += 2;
        pSrc += 3;
    }
    // -------------- end of buffer special handling (see text)
    pDst = (char*)pwDst;

    if (nLenSrc > 0)
    { // some left after last triple
        int n1 = (*pSrc & 0xfc) >> 2;
        int n2 = (*pSrc & 0x03) << 4;
        if (nLenSrc > 1)
        { // corrected.  Thanks to jprichey
            pSrc++;
            n2 |= (*pSrc & 0xf0) >> 4;
        }
        *pDst++ = Base64Digits[n1]; // encode at least 2 outputs
        *pDst++ = Base64Digits[n2];
        if (nLenSrc == 2)
        { // 2 src bytes left to encode, output xxx=
            int n3 = (*pSrc & 0x0f) << 2;
            pSrc++;
            n3 |= (*pSrc & 0xc0) >> 6;
            *pDst++ = Base64Digits[n3];
        }
        if (nLenSrc == 1)
        { // 1 src byte left to encode, output xx==
            *pDst++ = '=';
        }
        *pDst++ = '=';
    }
    // *pDst= 0; nLenOut++ // could terminate with NULL, here
    return (nLenOut);
}

// Utra-Fast base64-decoding, using 128KB lookup table
int FromBase64Fast(const BYTE* pSrc, int nLenSrc, char* pDst, int nLenDst)
{
    if (NULL == gpLookup16)
        SetupLookup16(); // see below
    int nLenOut = 0;
    if (nLenDst < ((nLenSrc / 4) - 1) * 3)
    {
        return (0); // (buffer too small)
    }
    int nLoopMax = (nLenSrc / 4) - 1;
    WORD* pwSrc = (WORD*)pSrc;
    for (int j = 0; j < nLoopMax; j++)
    {
        WORD s1 = gpLookup16[pwSrc[0]]; // translate two "digits" at once
        WORD s2 = gpLookup16[pwSrc[1]]; // ... and two more

        DWORD n32;
        n32 = s1; // xxxxxxxx xxxxxxxx xx111111 222222xx
        n32 <<= 10; // xxxxxxxx 11111122 2222xxxx xxxxxxxx
        n32 |= s2 >> 2; // xxxxxxxx 11111122 22223333 33444444

        BYTE b3 = (n32 & 0x00ff);
        n32 >>= 8; // in reverse (WORD order)
        BYTE b2 = (n32 & 0x00ff);
        n32 >>= 8;
        BYTE b1 = (n32 & 0x00ff);

        // *pDst++ = b1;  *pDst++ = b2;  *pDst++ = b3;  //slighly slower

        pDst[0] = b1; // slightly faster
        pDst[1] = b2;
        pDst[2] = b3;

        pwSrc += 2;
        pDst += 3;
    }
    nLenOut = ((nLenSrc / 4) - 1) * 3;

    //-------------------- special handling outside of loop for end
    WORD s1 = gpLookup16[pwSrc[0]];
    WORD s2 = gpLookup16[pwSrc[1]];

    DWORD n32;
    n32 = s1;
    n32 <<= 10;
    n32 |= s2 >> 2;

    BYTE b3 = (n32 & 0x00ff);
    n32 >>= 8;
    BYTE b2 = (n32 & 0x00ff);
    n32 >>= 8;
    BYTE b1 = (n32 & 0x00ff);

    // add that code to fix the length error
    // when the encoded string ends on '=' or '==' the final length should be
    // decrement
    BYTE bb2 = pSrc[nLenSrc - 2];
    BYTE bb3 = pSrc[nLenSrc - 1];

    if (nLenOut >= nLenDst)
        return (0); // error
    *pDst++ = b1;
    nLenOut++;

    if (bb2 != 61)
    {
        if (nLenOut >= nLenDst)
            return (0); // error
        *pDst++ = b2;
        nLenOut++;
    }
    if (bb3 != 61)
    {
        if (nLenOut >= nLenDst)
            return (0); // error
        *pDst++ = b3;
        nLenOut++;
    }

    return (nLenOut);
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
    print_info(tmp);
}

#define ENC_FLOAT4_LENGTH_B64 BASE64_ENCODE_OUT_SIZE(ENC_FLOAT4_LENGTH)

TEEInvoker::~TEEInvoker() {
    freeBuffer(req_buffer);
}
#define RETRY_FAILED 10086
int try_replay_request(void *req_buffer, FILE *read_file_ptr);
int replay_request(void *req){
    int ret = -RETRY_FAILED, cnt = 0;
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
        print_info("no valid record file found. error\n");
        return -1; // not valid record file found. error.
    }else {
        return try_replay_request(req, read_file_ptr);
    }
}

int try_replay_request(void *req_buffer, FILE *read_file_ptr){
    BaseRequest *req_control = static_cast<BaseRequest *>(req_buffer);
    int reqType = req_control->reqType;
    if(reqType>= 101 
    && reqType<= 109){
        int op;
        char b64_left[ENC_FLOAT4_LENGTH_B64],b64_right[ENC_FLOAT4_LENGTH_B64],b64_res[ENC_FLOAT4_LENGTH_B64];
        memset(b64_left, 0, sizeof(b64_left));
        memset(b64_right, 0, sizeof(b64_right));
        memset(b64_res, 0, sizeof(b64_res));
        EncFloat left,right,res;
        int cmp, resp;
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
        }else {
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
                print_info("not correct record file\n");
                // char tmp[100];
                // sprintf(tmp, "retry, diff is %d, %d, %d\n", op, 
                //     memcmp(&left, &req->left, ENC_FLOAT4_LENGTH),
                //     memcmp(&right, &req->right, ENC_FLOAT4_LENGTH));
                // print_info(tmp);
                // _print_hex("left b64", &b64_left, ENC_FLOAT4_LENGTH_B64);
                // _print_hex("left", &left, ENC_FLOAT4_LENGTH);
                // _print_hex("left-real", &req->left, ENC_FLOAT4_LENGTH);
                // _print_hex("right b64", &b64_right, ENC_FLOAT4_LENGTH_B64);
                // _print_hex("right", &right, ENC_FLOAT4_LENGTH);
                // _print_hex("right-real", &req->right, ENC_FLOAT4_LENGTH); 

                return -RETRY_FAILED;
            }
            req->res = res;
        }
        // print_info("get result from log\n");
        return resp;
    }

}

int TEEInvoker::sendRequest(Request *req) {
    int resp;
    req->serializeTo(req_buffer);
    BaseRequest *req_control = static_cast<BaseRequest *>(req_buffer);
    /* TODO write barrier */
    // print_info("REQUIEST sent");

    if(replayMode){
        if(req_control->reqType != CMD_FLOAT_ENC 
        && req_control->reqType != CMD_FLOAT_DEC
        && req_control->reqType >= 101
        && req_control->reqType <= 109
        ){
            int resp = replay_request(req_buffer);
            req->copyResultFrom(req_buffer);
            return resp;
        }
    }
    // static myTime time;
	// static double timer = 0,duration = 0;
	// time.tic();

    req_control->status = SENT;

    /* wait for status */
    while (req_control->status != DONE)
        YIELD_PROCESSOR;
    /* TODO read barrier */
    req->copyResultFrom(req_buffer);
    resp = req_control->resp;
    // char ch[1000];
    
    
    /* log */
    if(recordMode){
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
                fprintf(get_write_file_ptr(),"%d %s %s %d %d\n",
                    req_control->reqType, 
                    b64_left,
                    b64_right,
                    req->cmp,
                    req_control->resp);
            }
            else {
                EncFloatCalcRequestData *req = (EncFloatCalcRequestData *) req_buffer;
                EncFloat left;
                base64_encode((const unsigned char *)&req->left, ENC_FLOAT4_LENGTH, b64_left);
                base64_encode((const unsigned char *)&req->right, ENC_FLOAT4_LENGTH, b64_right);
                base64_encode((const unsigned char *)&req->res, ENC_FLOAT4_LENGTH, b64_res);
  
                fprintf(get_write_file_ptr(),"%d %s %s %s %d\n",
                    req->op, 
                    b64_left,
                    b64_right,
                    b64_res,
                    req_control->resp);
 
            }
            // flush_to_log_file((uint8_t *)ch, req_size);
        }
    }



    /* read-write barrier, no read move after this barrier, no write move before this barrier */
    
    req_control->status = NONE;
    free(req); /* free req to avoid mem leak */ 
    // time.toc();
    // duration = time.getDuration() / 1000000;
    // timer += duration;
    // if(req_control->reqType == CMD_FLOAT_DEC || req_control->reqType == CMD_INT_DEC){
    //     print_info("shit");
    //     char ch[1000];	
    //     sprintf(ch, "total duration in ms: sum %f ", timer );
    //     print_info(ch);
    // }

    return resp;
}

extern FILE *plain_file;
void exit_handler(){
    // print_info("EXIT handler called\n");
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
    // print_info("buffer got");
    atexit(exit_handler);
}
