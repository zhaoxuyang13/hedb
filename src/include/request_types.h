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
typedef struct
{
    volatile ReqStat status;
    int reqType;
    int resp;
} BaseRequest;

/* enc and dec req are for debug purpose. */
#define DEFINE_ENCTYPE_ENC_ReqData(enc_type, plain_type) \
typedef struct {           \
    BaseRequest common;    \
    plain_type plaintext;         \
    enc_type ciphertext;   \
} enc_type##EncRequestData;

DEFINE_ENCTYPE_ENC_ReqData(EncInt,int);
DEFINE_ENCTYPE_ENC_ReqData(EncFloat,float);
DEFINE_ENCTYPE_ENC_ReqData(EncTimestamp,int64_t);
DEFINE_ENCTYPE_ENC_ReqData(EncStr,PlainStr);

#define DEFINE_ENCTYPE_DEC_ReqData(enc_type,plain_type) \
typedef struct {             \
    BaseRequest common;      \
    enc_type ciphertext;     \
    plain_type plaintext;           \
} enc_type##DecRequestData;


DEFINE_ENCTYPE_DEC_ReqData(EncInt, int);
DEFINE_ENCTYPE_DEC_ReqData(EncFloat, float);
DEFINE_ENCTYPE_DEC_ReqData(EncTimestamp, int64_t);
DEFINE_ENCTYPE_DEC_ReqData(EncStr, PlainStr);

#define DEFINE_ENCTYPE_CMP_ReqData(enc_type) \
typedef struct {            \
    BaseRequest common;     \
    enc_type left;            \
    enc_type right;           \
    int cmp;                \
} enc_type##CmpRequestData;

DEFINE_ENCTYPE_CMP_ReqData(EncInt);
DEFINE_ENCTYPE_CMP_ReqData(EncFloat);
DEFINE_ENCTYPE_CMP_ReqData(EncTimestamp);
DEFINE_ENCTYPE_CMP_ReqData(EncStr);

#define DEFINE_ENCTYPE_CALC_ReqData(enc_type) \
typedef struct {              \
    BaseRequest common;       \
    int op;                   \
    enc_type left;            \
    enc_type right;           \
    enc_type res;             \
} enc_type##CalcRequestData;

DEFINE_ENCTYPE_CALC_ReqData(EncInt);
DEFINE_ENCTYPE_CALC_ReqData(EncFloat);
DEFINE_ENCTYPE_CALC_ReqData(EncTimestamp);
DEFINE_ENCTYPE_CALC_ReqData(EncStr);

#define DEFINE_ENCTYPE_BULK_ReqData(enc_type) \
typedef struct {                \
    BaseRequest common;         \
    int bulk_size;              \
    enc_type items[BULK_SIZE];  \
    enc_type res;               \
} enc_type##BulkRequestData;

DEFINE_ENCTYPE_BULK_ReqData(EncInt);
DEFINE_ENCTYPE_BULK_ReqData(EncFloat);

#ifdef __cplusplus
}
#endif