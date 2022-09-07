#pragma once 
#include <stddef.h>
#include <defs.h>
#include <stdlib.h>
#include <request_types.h>

#ifdef __cplusplus
extern "C" {
#endif

int encrypt_bytes(uint8_t *pSrc, size_t src_len, uint8_t *pDst, size_t exp_dst_len);
int decrypt_bytes(uint8_t *pSrc, size_t src_len, uint8_t *pDst, size_t exp_dst_len);
int decrypt_bytes_para(uint8_t *pSrc, size_t src_len, uint8_t *pDst, size_t exp_dst_len);
extern volatile int decrypt_status;

#ifdef __cplusplus
}
#endif

