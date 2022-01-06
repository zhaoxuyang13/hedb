#pragma once 
#include <enc_types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ReqStatus
{
    NONE,
    SENT,
    DONE
} ReqStat;
typedef struct BaseRequest
{
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
#ifdef __cplusplus
}
#endif