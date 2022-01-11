#pragma once

#include <sgx/enclave.hpp>
#include <request_types.h>

typedef union {
    float val;
    unsigned char bytes[FLOAT4_LENGTH];
} union_float4;

int enc_float32_calc(EncFloatCalcRequestData *req);
int enc_float32_cmp(EncFloatCmpRequestData *req);
int enc_float32_bulk(EncFloatBulkRequestData *req);
