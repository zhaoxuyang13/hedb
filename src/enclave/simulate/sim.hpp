#pragma once 

#ifdef __cplusplus
extern "C" {
#endif

int encrypt_bytes(uint8_t *pSrc, size_t src_len, uint8_t *pDst, size_t exp_dst_len);
int decrypt_bytes(uint8_t *pSrc, size_t src_len, uint8_t *pDst, size_t exp_dst_len);

#ifdef __cplusplus
}
#endif

