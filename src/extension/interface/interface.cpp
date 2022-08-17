#include <interface.hpp>
#include <tee_interface.hpp>
#include <sync.h>
#include <extension_helper.hpp>
#include <timer.hpp>
#include <stdlib.h> // at exit
#include <stdio.h>
#define MAX_LOG_SIZE 50000

TEEInvoker *TEEInvoker::invoker = NULL;


uint8_t log_buffer[MAX_LOG_SIZE] ;
uint64_t current_log_size = 0;
FILE *write_ptr = 0;

void flush_to_log_file(uint8_t *buffer, uint64_t size){
    if (write_ptr == 0)
    {
        write_ptr = fopen("test.binlog","wb"); 
    }
    fwrite(buffer,size,1,write_ptr);
}
FILE *file_ptr = 0;
FILE *get_file_ptr(){
    if (file_ptr == 0)
    {
        file_ptr = fopen("test.binlog","w+"); 
    }
    return file_ptr;
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


#define ENC_FLOAT4_LENGTH_B64 45




TEEInvoker::~TEEInvoker() {
    freeBuffer(req_buffer);
}

int TEEInvoker::sendRequest(Request *req) {
    int resp;
    
    req->serializeTo(req_buffer);
    BaseRequest *req_control = static_cast<BaseRequest *>(req_buffer);
    /* TODO write barrier */
    // print_info("REQUIEST sent");

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
    /*
    if(req_control->reqType != CMD_FLOAT_ENC 
    && req_control->reqType != CMD_FLOAT_DEC
    && req_control->reqType >= 101
    && req_control->reqType <= 109
    ){
        char b64_left[ENC_FLOAT4_LENGTH_B64],b64_right[ENC_FLOAT4_LENGTH_B64],b64_res[ENC_FLOAT4_LENGTH_B64];
        if(req_control->reqType == CMD_FLOAT_CMP){
            EncFloatCmpRequestData *req = (EncFloatCmpRequestData *) req_buffer;
            ToBase64Fast((const unsigned char *)&req->left, ENC_FLOAT4_LENGTH, b64_left, ENC_FLOAT4_LENGTH_B64);
            ToBase64Fast((const unsigned char *)&req->right, ENC_FLOAT4_LENGTH, b64_right, ENC_FLOAT4_LENGTH_B64);
            fprintf(get_file_ptr(),"%d,%s,%s,%d,%d\n",
                req_control->reqType, 
                b64_left,
                b64_right,
                req->cmp,
                req_control->resp);
        }
        else {
            EncFloatCalcRequestData *req = (EncFloatCalcRequestData *) req_buffer;
            ToBase64Fast((const unsigned char *)&req->left, ENC_FLOAT4_LENGTH, b64_left, ENC_FLOAT4_LENGTH_B64);
            ToBase64Fast((const unsigned char *)&req->right, ENC_FLOAT4_LENGTH, b64_right, ENC_FLOAT4_LENGTH_B64);
            ToBase64Fast((const unsigned char *)&req->res, ENC_FLOAT4_LENGTH, b64_res, ENC_FLOAT4_LENGTH_B64);
            fprintf(get_file_ptr(),"%d,%s,%s,%s,%d\n",
                req->op, 
                b64_left,
                b64_right,
                b64_res,
                req_control->resp);
        }
        // flush_to_log_file((uint8_t *)ch, req_size);
    }
    */



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
    if(write_ptr != 0)
        fclose(write_ptr);
    if(plain_file != 0)
       fclose(plain_file); 
}


TEEInvoker::TEEInvoker() {
    // print_info("get shared buffer");
    req_buffer = getSharedBuffer(sizeof (EncIntBulkRequestData));
    // print_info("buffer got");
    atexit(exit_handler);
}
