#pragma once 

#include <enc_int_ops.h>
#include <enc_float_ops.h>
#include <enc_timestamp_ops.h>
#include <enc_text_ops.h>
#include <enc_types.h>
#include <request_types.h>
#include <defs.h>

#ifdef __cplusplus 
extern "C" {
#endif 

int handle_ops(BaseRequest *req);

#ifdef __cplusplus 
}
#endif 
