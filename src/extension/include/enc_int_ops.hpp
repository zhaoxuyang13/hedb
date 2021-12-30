#pragma once 

#ifdef __cplusplus
extern "C" {
#endif
#include <enc_types.h>

int enc_int_add(EncInt *int1, EncInt *int2, EncInt *res);
int enc_int_sub(EncInt *int1, EncInt *int2, EncInt *res);
int enc_int_mult(EncInt *int1, EncInt *int2, EncInt *res);
int enc_int_div(EncInt *int1, EncInt *int2, EncInt *res);
int enc_int_pow(EncInt *int1, EncInt *int2, EncInt *res);
int enc_int_mod(EncInt *int1, EncInt *int2, EncInt *res);
int enc_int_cmp(EncInt *int1, EncInt *int2, int *res);
int enc_int_encrypt(int pSrc, EncInt *pDst);
int enc_int_decrypt(EncInt *pSrc, int *pDst);
int enc_int_sum_bulk(size_t bulk_size, EncInt *arg1, EncInt *res);
#ifdef __cplusplus
}
#endif
