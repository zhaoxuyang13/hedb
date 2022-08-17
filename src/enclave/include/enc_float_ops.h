#pragma once

#if defined(TEE_SGX)
#include <sgx/enclave.hpp>
#elif defined(TEE_TZ)
#include <trustzone/ops_ta.h>
#endif

#include <request_types.h>
#include <kv.h>

typedef union {
    float val;
    unsigned char bytes[FLOAT4_LENGTH];
} union_float4;

int enc_float32_calc(EncFloatCalcRequestData *req);
int enc_float32_cmp(EncFloatCmpRequestData *req);
int enc_float32_bulk(EncFloatBulkRequestData *req);
int enc_float32_eval_expr(EncFloatEvalExprRequestData *req);
