#pragma once 
#include <defs.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
// #define DECLARE_PG_FUNC(func) extern "C" { PG_FUNCTION_INFO_V1(func); } 


#define DEFINE_ENCTYPE(type_name ,data_size) \
typedef struct {            \
    uint8_t iv[IV_SIZE];    \
    uint8_t data[data_size]; \
    uint8_t tag[TAG_SIZE];  \
} type_name;
 
/* enc_int type */

DEFINE_ENCTYPE(EncInt, INT32_LENGTH);
DEFINE_ENCTYPE(EncFloat, FLOAT4_LENGTH);
DEFINE_ENCTYPE(EncTimestamp, TIMESTAMP_LENGTH);
DEFINE_ENCTYPE(EncStr, STRING_LENGTH);

typedef struct 
{
    uint8_t data[STRING_LENGTH];
}PlainStr;