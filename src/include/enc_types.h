#pragma once 
#include <defs.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
// #define DECLARE_PG_FUNC(func) extern "C" { PG_FUNCTION_INFO_V1(func); } 


#define DEFINE_ENCTYPE(type_name ,data_size) \
typedef struct {            \
    uint8_t iv[IV_SIZE];    \
    uint8_t tag[TAG_SIZE];  \
    uint8_t data[data_size]; \
} type_name;
 
/* enc_int type */

DEFINE_ENCTYPE(EncInt, INT32_LENGTH);
DEFINE_ENCTYPE(EncFloat, FLOAT4_LENGTH);
DEFINE_ENCTYPE(EncTimestamp, TIMESTAMP_LENGTH);
DEFINE_ENCTYPE(EncCStr, STRING_LENGTH);

typedef struct 
{
    uint32_t len;
    EncCStr enc_cstr;
}EncStr;

typedef struct 
{
    uint32_t len;
    uint8_t data[STRING_LENGTH];
}Str;

#define ENCSTRLEN(len) (sizeof(uint32_t) + IV_SIZE + TAG_SIZE + len)