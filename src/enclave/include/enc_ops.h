#pragma once 

#include <enc_int_ops.h>
#include <enc_float_ops.h>
#include <enc_timestamp_ops.h>
#include <enc_text_ops.h>
#include <request_types.h>
#include <defs.h>

#ifdef __cplusplus 
extern "C" {
#endif 

int handle_ops(BaseRequest *req);

#ifdef __cplusplus 
}
#endif 

#ifndef ENABLE_KV
#define float_map_find(a, b, c) (*c = false)
#define text_map_find(a, b, c) (*c = false)
#define float_map_insert(...) ;
#define text_map_insert(...) ;
#endif 