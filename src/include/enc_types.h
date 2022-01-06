#pragma once 
#include <defs.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
// #define DECLARE_PG_FUNC(func) extern "C" { PG_FUNCTION_INFO_V1(func); } 

/* enc_int type */

typedef struct EncInt
{
    uint8_t iv[IV_SIZE];
    uint8_t data[INT32_LENGTH]; 
    uint8_t tag[TAG_SIZE];
    /* may contain other fields like ope */
} EncInt;

// typedef struct request
// {
    /* data */
// };
