#pragma once

// #include "tools/like_match.h"
// #include <string.h>

#if defined(TEE_SGX)
#include <sgx/enclave.hpp>
#elif defined(TEE_TZ) 
#include <trustzone/ops_ta.h>
#endif
#include <request_types.h>
#include <kv.h>


int enc_text_cmp(EncStrCmpRequestData *req);
int enc_text_like(EncStrLikeRequestData *req);
int enc_text_concatenate(EncStrCalcRequestData *req);
int enc_text_substring(SubstringRequestData *req);

