#pragma once

#if defined(TEE_SGX)
#include <sgx/enclave.hpp>
#elif defined(TEE_TZ) 
#include <trustzone/ops_ta.h>
#endif


int enc_int32_add(uint8_t* int1,
                  size_t int1_len,
                  uint8_t* int2,
                  size_t int2_len,
                  uint8_t* int3,
                  size_t int3_len);
int enc_int32_cmp(uint8_t* int1,
                  size_t int1_len,
                  uint8_t* int2,
                  size_t int2_len,
                  uint8_t* result,
                  size_t res_len);
int enc_int32_sub(uint8_t* int1,
                  size_t int1_len,
                  uint8_t* int2,
                  size_t int2_len,
                  uint8_t* result,
                  size_t res_len);
int enc_int32_mult(uint8_t* int1,
                   size_t int1_len,
                   uint8_t* int2,
                   size_t int2_len,
                   uint8_t* result,
                   size_t res_len);
int enc_int32_div(uint8_t* int1,
                  size_t int1_len,
                  uint8_t* int2,
                  size_t int2_len,
                  uint8_t* result,
                  size_t res_len);
int enc_int32_mod(uint8_t* int1,
                  size_t int1_len,
                  uint8_t* int2,
                  size_t int2_len,
                  uint8_t* result,
                  size_t res_len);
int enc_int32_pow(uint8_t* int1,
                  size_t int1_len,
                  uint8_t* int2,
                  size_t int2_len,
                  uint8_t* result,
                  size_t res_len);
int enc_int32_sum_bulk(uint8_t* arg1,
                       size_t arg1_len,
                       uint8_t* arg2,
                       size_t arg2_len,
                       uint8_t* result,
                       size_t res_len);

// int enc_int32_add(EncIntCalcRequestData *req);
// int enc_int32_sub(EncIntCalcRequestData *req);
// int enc_int32_mult(EncIntCalcRequestData *req);
// int enc_int32_div(EncIntCalcRequestData *req;
// int enc_int32_mod(EncIntCalcRequestData *req);
// int enc_int32_pow(EncIntCalcRequestData *req);
// int enc_int32_cmp(EncIntCmpRequestData *req);
// int enc_int32_sum_bulk(EncIntBulkRequestData *req);
