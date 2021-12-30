#pragma once 
#include <request_types.h>

typedef enum ReqStatus
{
    NONE,
    SENT,
    DONE
} ReqStat;
typedef struct BaseRequest
{
    static const int max_buffer_size = 65536;
    volatile ReqStat status;
    int reqType;
    int resp;
} BaseRequest;
typedef struct EncIntCmpRequestData {
    BaseRequest common;
    EncInt left;
    EncInt right;
    int cmp;
}EncIntCmpRequestData;
typedef struct EncIntCalcRequestData {
    BaseRequest common;
    int op;
    EncInt left;
    EncInt right;
    EncInt res;
}EncIntCalcRequestData;
typedef struct EncIntBulkRequestData {
    BaseRequest common;
    int bulk_size;
    EncInt items[BULK_SIZE];
    EncInt res;
}EncIntBulkRequestData;
typedef struct EncIntEncRequestData {
    BaseRequest common;
    int plaintext;
    EncInt ciphertext;
}EncIntEncRequestData;
typedef struct EncIntDecRequestData {
    BaseRequest common;
    EncInt ciphertext;
    int plaintext;
}EncIntDecRequestData;