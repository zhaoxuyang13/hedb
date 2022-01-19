/*
 * Copyright (c) 2016, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <arm64_user_sysreg.h>
#include <enc_float32_ops.h>
#include <enc_int32_ops.h>
#include <enc_text_ops.h>
#include <enc_timestamp_ops.h>
#include <ops_ta.h>
#include <string.h>
#include <string_ext.h>
#include <util.h>
#include <assert.h> // for debugging
#include "crypto.h"

// #define __TA_PROFILE 
/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */
TEE_Result TA_CreateEntryPoint(void)
{
	DMSG("ops TA_CREATE has been called");

	return TEE_SUCCESS;
}

/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TA_DestroyEntryPoint(void)
{
	DMSG("ops TA_DESTROY has been called");
}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA. In this function you will normally do the global initialization for the
 * TA.
 */
TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
									TEE_Param __maybe_unused params[4],
									void __maybe_unused **sess_ctx)
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Unused parameters */
	(void)&params;
	(void)&sess_ctx;

	/*
	 * The DMSG() macro is non-standard, TEE Internal API doesn't
	 * specify any means to logging from a TA.
	 */
	IMSG("Hello World!\n");

	/* If return value != TEE_SUCCESS the session will not be created. */
	return TEE_SUCCESS;
}


/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TA_CloseSessionEntryPoint(void __maybe_unused *sess_ctx)
{
	(void)&sess_ctx; /* Unused parameter */
	IMSG("ops Goodbye!\n");
}

static TEE_Result inc_value(uint32_t param_types,
							TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	static int counter = 0;
	counter++;
	if (counter % 100000 == 0)
	{
		DMSG("%d\n", counter);
	}

	// uint64_t duration =0,timer = 0;
	// uint64_t start,end;
	// uint32_t freq = read_cntfrq();

	// duration = (end - start) * 1000000 / freq;
	// timer += duration;
	// DMSG("duration in us %d",duration);

	params[0].value.a++;
	return TEE_SUCCESS;
}

static TEE_Result gcm_demo(uint32_t param_types,
						   TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INOUT,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE);
	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	TEE_Result res = TEE_SUCCESS;

	uint32_t in_sz = 0;
	uint32_t out_sz = 0;
	DMSG("gcm demo called \n");
	char buffer[1000];
	int a = 0x1234;
	memcpy(buffer, &a, INT32_LENGTH);

	in_sz = INT32_LENGTH;
	res = gcm_encrypt(buffer, in_sz, buffer + INT32_LENGTH, &out_sz);
	DMSG("in %d,struct %d,out %d", in_sz, *(int *) buffer,out_sz);
	if (res == TEE_SUCCESS)
	{
		assert(out_sz == ENC_INT32_LENGTH);
	}

//	_print_hex("",buffer,INT32_LENGTH +  ENC_INT32_LENGTH);
	memset(buffer, 0, INT32_LENGTH);
	in_sz = ENC_INT32_LENGTH;
	out_sz = INT32_LENGTH;
	res = gcm_decrypt(buffer + INT32_LENGTH, in_sz, buffer, &out_sz);
	DMSG("decrypt size %d, result %d\n",out_sz, *((int*) buffer));
	return TEE_SUCCESS;
}
static TEE_Result dec_value(uint32_t param_types,
							TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INOUT,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE);
	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;
	request_t *req = (request_t *)params[0].memref.buffer;
	assert(params[0].memref.size == sizeof(request_t));
	assert(req->ocall_index = 233);
	int counter = 0;

	gcm_demo(param_types,params);
	
	size_t src_len = 0, src2_len = 0, src3_len = 0, dst_len = 0;
    uint8_t src1[INPUT_BUFFER_SIZE];
    uint8_t src2[INPUT_BUFFER_SIZE];
    uint8_t dst[INPUT_BUFFER_SIZE];
    uint8_t in1[ENC_INT32_LENGTH], in2[ENC_INT32_LENGTH];
    int buf_pos = 0;
	// bool touched[300];
	// for(int i = 0; i < 300 ; i ++)
	// 	touched[i] = false;
	uint64_t duration =0,timer = 0;
	uint64_t start,end;
	uint32_t freq = read_cntfrq();
	while (true)
	{
		if (req->is_done == -1)
		{
			asm volatile("dsb ld" ::: "memory");
#ifdef __TA_PROFILE
			start = read_cntpct();
#endif
			counter ++;
			if (counter % 10000 == 0)
				DMSG("counter %d", counter++);

			// if(!touched[req->ocall_index]){
			// 	touched[req->ocall_index] = true;
			// 	printf("touched: ");
			// 	for (int i = 0; i < 300; i++)
			// 		if (touched[i])
			// 			printf("%d,",i);
			// 	printf(", current one is %d\n",req->ocall_index);
			// }
			switch (req->ocall_index)
			{
			case CMD_INT64_ENC:
				req->resp = encrypt_bytes(req->buffer,
										  INT32_LENGTH,
										  req->buffer + INT32_LENGTH,
										  ENC_INT32_LENGTH);
				// enc_int_counter++;
				// INFO("encrypt: %x\n",*((unsigned int *) req->buffer));
				break;
			case CMD_INT64_DEC:
				req->resp = decrypt_bytes(req->buffer,
										  ENC_INT32_LENGTH,
										  req->buffer + ENC_INT32_LENGTH,
										  INT32_LENGTH);
				// INFO("decrypt:%x -> %x\n",*((unsigned int *) req->buffer) ,*((unsigned int *) (req->buffer+ENC_INT32_LENGTH)));
				// dec_int_counter++;
				break;
			case CMD_INT64_PLUS:
				req->resp = enc_int32_add(req->buffer,
										  ENC_INT32_LENGTH,
										  req->buffer + ENC_INT32_LENGTH,
										  ENC_INT32_LENGTH,
										  req->buffer + ENC_INT32_LENGTH + ENC_INT32_LENGTH,
										  ENC_INT32_LENGTH);
				// INFO("add result: %x\n", *((unsigned int *) (req->buffer+ ENC_INT32_LENGTH * 2)));
				break;
			case CMD_INT64_MINUS:
				req->resp = enc_int32_sub(req->buffer,
										  ENC_INT32_LENGTH,
										  req->buffer + ENC_INT32_LENGTH,
										  ENC_INT32_LENGTH,
										  req->buffer + ENC_INT32_LENGTH + ENC_INT32_LENGTH,
										  ENC_INT32_LENGTH);
				break;

			case CMD_INT64_MULT:
				req->resp = enc_int32_mult(req->buffer,
										   ENC_INT32_LENGTH,
										   req->buffer + ENC_INT32_LENGTH,
										   ENC_INT32_LENGTH,
										   req->buffer + ENC_INT32_LENGTH + ENC_INT32_LENGTH,
										   ENC_INT32_LENGTH);
				break;

			case CMD_INT64_DIV:
				req->resp = enc_int32_div(req->buffer,
										  ENC_INT32_LENGTH,
										  req->buffer + ENC_INT32_LENGTH,
										  ENC_INT32_LENGTH,
										  req->buffer + ENC_INT32_LENGTH + ENC_INT32_LENGTH,
										  ENC_INT32_LENGTH);
				break;

			case CMD_INT64_EXP:
				req->resp = enc_int32_pow(req->buffer,
										  ENC_INT32_LENGTH,
										  req->buffer + ENC_INT32_LENGTH,
										  ENC_INT32_LENGTH,
										  req->buffer + ENC_INT32_LENGTH + ENC_INT32_LENGTH,
										  ENC_INT32_LENGTH);
				break;

			case CMD_INT64_MOD:
				req->resp = enc_int32_mod(req->buffer,
										  ENC_INT32_LENGTH,
										  req->buffer + ENC_INT32_LENGTH,
										  ENC_INT32_LENGTH,
										  req->buffer + ENC_INT32_LENGTH + ENC_INT32_LENGTH,
										  ENC_INT32_LENGTH);
				break;
			case CMD_INT64_CMP:
				req->resp = enc_int32_cmp(req->buffer,
										  ENC_INT32_LENGTH,
										  req->buffer + ENC_INT32_LENGTH,
										  ENC_INT32_LENGTH,
										  req->buffer + 2 * ENC_INT32_LENGTH,
										  INT32_LENGTH);
				break;
			case CMD_INT32_SUM_BULK:
				memcpy(&src_len, req->buffer, INT32_LENGTH);
				req->resp = enc_int32_sum_bulk(
					req->buffer,
					INT32_LENGTH,
					req->buffer + INT32_LENGTH,
					src_len * ENC_INT32_LENGTH,
					req->buffer + (src_len)*ENC_INT32_LENGTH + INT32_LENGTH,
					ENC_INT32_LENGTH);
				break;
			case CMD_FLOAT4_PLUS:
				req->resp = enc_float32_add(req->buffer,
											ENC_FLOAT4_LENGTH,
											req->buffer + ENC_FLOAT4_LENGTH,
											ENC_FLOAT4_LENGTH,
											req->buffer + 2 * ENC_FLOAT4_LENGTH,
											ENC_FLOAT4_LENGTH);
				break;

			case CMD_FLOAT4_MINUS:
				req->resp = enc_float32_sub(req->buffer,
											ENC_FLOAT4_LENGTH,
											req->buffer + ENC_FLOAT4_LENGTH,
											ENC_FLOAT4_LENGTH,
											req->buffer + 2 * ENC_FLOAT4_LENGTH,
											ENC_FLOAT4_LENGTH);
				break;

			case CMD_FLOAT4_MULT:
				req->resp = enc_float32_mult(req->buffer,
											 ENC_FLOAT4_LENGTH,
											 req->buffer + ENC_FLOAT4_LENGTH,
											 ENC_FLOAT4_LENGTH,
											 req->buffer + 2 * ENC_FLOAT4_LENGTH,
											 ENC_FLOAT4_LENGTH);
				break;

			case CMD_FLOAT4_DIV:
				req->resp = enc_float32_div(req->buffer,
											ENC_FLOAT4_LENGTH,
											req->buffer + ENC_FLOAT4_LENGTH,
											ENC_FLOAT4_LENGTH,
											req->buffer + 2 * ENC_FLOAT4_LENGTH,
											ENC_FLOAT4_LENGTH);
				break;

			case CMD_FLOAT4_EXP:
				req->resp = enc_float32_pow(req->buffer,
											ENC_FLOAT4_LENGTH,
											req->buffer + ENC_FLOAT4_LENGTH,
											ENC_FLOAT4_LENGTH,
											req->buffer + 2 * ENC_FLOAT4_LENGTH,
											ENC_FLOAT4_LENGTH);
				break;

			case CMD_FLOAT4_MOD:
				req->resp = enc_float32_mod(req->buffer,
											ENC_FLOAT4_LENGTH,
											req->buffer + ENC_FLOAT4_LENGTH,
											ENC_FLOAT4_LENGTH,
											req->buffer + 2 * ENC_FLOAT4_LENGTH,
											ENC_FLOAT4_LENGTH);
				break;

			case CMD_FLOAT4_CMP:
				req->resp = enc_float32_cmp(req->buffer,
											ENC_FLOAT4_LENGTH,
											req->buffer + ENC_FLOAT4_LENGTH,
											ENC_FLOAT4_LENGTH,
											req->buffer + 2 * ENC_FLOAT4_LENGTH,
											INT32_LENGTH);
				break;

				// case CMD_FLOAT4_SUM_BULK:
				// 	memcpy(&src_len, req->buffer, INT32_LENGTH);
				// 	req->resp = enc_float32_sum_bulk(
				// 		req->buffer,
				// 		INT32_LENGTH,
				// 		req->buffer + INT32_LENGTH,
				// 		src_len * ENC_FLOAT4_LENGTH,
				// 		req->buffer + (src_len)*ENC_FLOAT4_LENGTH + INT32_LENGTH,
				// 		ENC_FLOAT4_LENGTH);
				// 	break;

			case CMD_FLOAT4_ENC:
				req->resp = encrypt_bytes(req->buffer,
										  FLOAT4_LENGTH,
										  req->buffer + FLOAT4_LENGTH,
										  ENC_FLOAT4_LENGTH);
				break;

			case CMD_FLOAT4_DEC:
				req->resp = decrypt_bytes(req->buffer,
										  ENC_FLOAT4_LENGTH,
										  req->buffer + ENC_FLOAT4_LENGTH,
										  FLOAT4_LENGTH);
				break;
			case CMD_STRING_CMP:
				memcpy(&src_len, req->buffer, INT32_LENGTH);
				memcpy(&src2_len, req->buffer + INT32_LENGTH + src_len, INT32_LENGTH);
		
				req->resp = enc_text_cmp(req->buffer + INT32_LENGTH,
											src_len,
											req->buffer + INT32_LENGTH + src_len + INT32_LENGTH,
											src2_len,
											req->buffer + 2 * INT32_LENGTH + src_len + src2_len,
											INT32_LENGTH);

				break;

			case CMD_STRING_LIKE:
				memcpy(&src_len, req->buffer + buf_pos, INT32_LENGTH);
				buf_pos += INT32_LENGTH;

				memcpy(src1, req->buffer + buf_pos, src_len);
				buf_pos += src_len;

				memcpy(&src2_len, req->buffer + buf_pos, INT32_LENGTH);
				buf_pos += INT32_LENGTH;

				memcpy(src2, req->buffer + buf_pos, src2_len);
				buf_pos += src2_len;

				req->resp = enc_text_like(
					src1, src_len, src2, src2_len, req->buffer + buf_pos, INT32_LENGTH);
				buf_pos = 0;
				break;

			case CMD_STRING_ENC:
				memcpy(&src_len, req->buffer, INT32_LENGTH);
				dst_len = src_len + IV_SIZE + TAG_SIZE;

				memcpy(src1, req->buffer + INT32_LENGTH, src_len);

				memcpy(req->buffer + INT32_LENGTH + src_len, &dst_len, INT32_LENGTH);
				req->resp = encrypt_bytes(
					src1, src_len, req->buffer + src_len + 2 * INT32_LENGTH, dst_len);
				break;

			case CMD_STRING_DEC:
				memcpy(&src_len, req->buffer, INT32_LENGTH);
				dst_len = src_len - IV_SIZE - TAG_SIZE;
				memcpy(src1, req->buffer + INT32_LENGTH, src_len);
				req->resp = decrypt_bytes(src1, src_len, dst, dst_len);
				memcpy(req->buffer + INT32_LENGTH + src_len, &dst_len, INT32_LENGTH);
				memcpy(req->buffer + src_len + 2 * INT32_LENGTH, dst, dst_len);
				break;

			case CMD_STRING_SUBSTRING: /* write src1, src2 will trigger write permission fault.*/
				memcpy(&src_len, req->buffer + buf_pos, INT32_LENGTH);
				buf_pos += INT32_LENGTH;

				memcpy(src1, req->buffer + buf_pos, src_len);
				buf_pos += src_len;

				memcpy(&src2_len, req->buffer + buf_pos, INT32_LENGTH);
				buf_pos += INT32_LENGTH;

				memcpy(in1, req->buffer + buf_pos, src2_len);
				buf_pos += src2_len;

				memcpy(&src3_len, req->buffer + buf_pos, INT32_LENGTH);
				buf_pos += INT32_LENGTH;

				memcpy(in2, req->buffer + buf_pos, src3_len);
				buf_pos += src3_len;

				memcpy(&dst_len, req->buffer + buf_pos, INT32_LENGTH);
				buf_pos += INT32_LENGTH;

				memcpy(dst, req->buffer + buf_pos, dst_len);
				buf_pos += dst_len;

				req->resp = enc_text_substring(
					src1, src_len, in1, src2_len, in2, src3_len, dst, &dst_len);

				memcpy(req->buffer + buf_pos, &dst_len, INT32_LENGTH);
				buf_pos += INT32_LENGTH;

				memcpy(req->buffer + buf_pos, dst, dst_len);
				buf_pos = 0;
				break;

			case CMD_STRING_CONCAT: 
				memcpy(&src_len, req->buffer + buf_pos, INT32_LENGTH);
				buf_pos += INT32_LENGTH;

				// memcpy(src1, req->buffer + buf_pos, src_len);
				buf_pos += src_len;

				memcpy(&src2_len, req->buffer + buf_pos, INT32_LENGTH);
				buf_pos += INT32_LENGTH;

				// memcpy(src2, req->buffer + buf_pos, src2_len);
				dst_len = src_len + src2_len - IV_SIZE - TAG_SIZE;

				req->resp = enc_text_concatenate(req->buffer + INT32_LENGTH, src_len, 
													req->buffer + INT32_LENGTH * 2 + src_len, src2_len, 
													dst, dst_len);
				memcpy(req->buffer + buf_pos, &dst_len, INT32_LENGTH);
				buf_pos += INT32_LENGTH;

				memcpy(req->buffer + buf_pos, dst, dst_len);
				buf_pos = 0;
				break;	

            case CMD_TIMESTAMP_EXTRACT_YEAR:
                req->resp = enc_timestamp_extract_year(req->buffer,
                                                       ENC_TIMESTAMP_LENGTH,
                                                       req->buffer + ENC_TIMESTAMP_LENGTH,
                                                       ENC_INT32_LENGTH);
                break;

            case CMD_TIMESTAMP_CMP:
                req->resp = enc_timestamp_cmp(req->buffer,
                                              TIMESTAMP_LENGTH,
                                              req->buffer + ENC_TIMESTAMP_LENGTH,
                                              ENC_TIMESTAMP_LENGTH,
                                              req->buffer + 2 * ENC_TIMESTAMP_LENGTH,
                                              INT32_LENGTH);
                break;

            case CMD_TIMESTAMP_ENC:
                req->resp = encrypt_bytes(req->buffer,
                                          TIMESTAMP_LENGTH,
                                          req->buffer + TIMESTAMP_LENGTH,
                                          ENC_TIMESTAMP_LENGTH);
                break;

            case CMD_TIMESTAMP_DEC:
                req->resp = decrypt_bytes(req->buffer,
                                          ENC_TIMESTAMP_LENGTH,
                                          req->buffer + ENC_TIMESTAMP_LENGTH,
                                          TIMESTAMP_LENGTH);
                break;
			 
			default:
				IMSG("Unimplemented Command: 0x%x \n", req->ocall_index);
				break;
			}
			if (req->resp != 0)
			{
				DMSG("TA error %d, %d\n",req->resp,counter);
			}
			// __sync_synchronize();
			asm volatile("dsb st" ::: "memory");
			req->is_done = 1;

#ifdef __TA_PROFILE			
			end = read_cntpct();
			duration = (end - start) * 1000000 / freq;
			timer += duration;
			DMSG("duration in us %d, end %d, start %d, freq %d,, avg %d",duration,end,start,freq, timer/counter);
#endif
		}
		
		if (req->is_done == 233)
		{
			DMSG("TA Exit %d\n", counter);
			break;
		}
	}


	return TEE_SUCCESS;
}

/* Decrypts byte array by aesgcm mode
 @input: uint8_t array - pointer to encrypted byte array
         size_t - length of encrypted  array
         uint8_t array - pointer to decrypted array
         size_t - length of decrypted array (length of array -
 IV_SIZE - TAG_SIZE)
 @return:
    * SGX_error, if there was an error during encryption/decryption
    0, otherwise
*/
int decrypt_bytes(uint8_t *pSrc, size_t src_len, uint8_t *pDst, size_t exp_dst_len)
{
	
	// _print_hex("dec ", pSrc, src_len);

	int dst_len = exp_dst_len, resp = 0;
	// DMSG("before dec %x %d %x %d",pSrc,src_len, pDst, dst_len);

	// uint64_t duration =0,timer = 0;
	// uint64_t start,end;
	// uint32_t freq = read_cntfrq();
	// start = read_cntpct();

	// for (size_t i = 0; i < 1000; i++)
	// {


	resp = gcm_decrypt(pSrc, src_len, pDst, &dst_len);
	// DMSG("after dec");
	assert(dst_len == exp_dst_len);

	// }

	// end= read_cntpct();
	// duration = (end - start) * 1000000 / freq;
	// timer += duration;
	// DMSG("1000 dec duration in us %d",duration);


	// memset(pDst, 0, dst_len);
	// memcpy(pDst, pSrc, dst_len);
	if (resp !=0)
	{
		_print_hex("dec from", pSrc, src_len);
		_print_hex("dec to ", pDst, dst_len);
	}
	
	return resp;
}

/* Encrypts byte array by aesgcm mode
 @input: uint8_t array - pointer to a byte array
         size_t - length of the array
         uint8_t array - pointer to result array
         size_t - length of result array (IV_SIZE + length of array +
 TAG_SIZE)
 @return:
    * SGX_error, if there was an error during encryption/decryption
    0, otherwise
*/
int encrypt_bytes(uint8_t *pSrc, size_t src_len, uint8_t *pDst, size_t exp_dst_len)
{

	int dst_len = exp_dst_len, resp = 0;

	// uint64_t duration =0,timer = 0;
	// uint64_t start,end;
	// uint32_t freq = read_cntfrq();
	// start = read_cntpct();

	// for (size_t i = 0; i < 1000; i++)
	// {

		// DMSG("before enc %x %d %x %d",pSrc,src_len, pDst, dst_len);
	resp = gcm_encrypt(pSrc, src_len, pDst, &dst_len);
	// DMSG("after enc");
	assert(dst_len == exp_dst_len);

	// }

	// end= read_cntpct();
	// duration = (end - start) * 1000000 / freq;
	// timer += duration;
	// DMSG("1000 enc duration in us %d",duration);

	/*uncomment below to disable encryption*/
	// memset(pDst, 0, dst_len);
	// memcpy(pDst, pSrc, src_len);	
	if (resp !=0)
	{
		_print_hex("enc ", pSrc, src_len);
		_print_hex("enc to  ", pDst, dst_len);
	}
	return resp;
}

static TEE_Result proccess_ops(uint32_t param_types,
							   TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INOUT,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE);

	// DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;
	// static long long count = 0, enc_int_counter = 0, dec_int_counter = 0, cmp_int_counter = 0;

	request_t *req = (request_t *)params[0].memref.buffer;

	if (req == NULL || params[0].memref.size != sizeof(request_t))
	{
		IMSG("invalid input: 0x%p, %ld\n", req, req->ocall_index);
		return TEE_ERROR_BAD_PARAMETERS;
	}

	static int counter = 0;
	counter++;
	if (counter % 10000 == 0)
	{
		DMSG("%d\n", counter);
	}
#ifdef __TA_PROFILE
	uint64_t start,end,duration,timer;
	uint32_t freq = read_cntfrq();
	start = read_cntpct();
#endif
	switch (req->ocall_index)
	{
	case CMD_INT64_ENC:
		req->resp = encrypt_bytes(req->buffer,
								  INT32_LENGTH,
								  req->buffer + INT32_LENGTH,
								  ENC_INT32_LENGTH);
		// enc_int_counter++;
		// INFO("encrypt: %x\n",*((unsigned int *) req->buffer));
		break;
	case CMD_INT64_DEC:
		req->resp = decrypt_bytes(req->buffer,
								  ENC_INT32_LENGTH,
								  req->buffer + ENC_INT32_LENGTH,
								  INT32_LENGTH);
		// INFO("decrypt:%x -> %x\n",*((unsigned int *) req->buffer) ,*((unsigned int *) (req->buffer+ENC_INT32_LENGTH)));
		// dec_int_counter++;
		break;
	case CMD_INT64_PLUS:
		req->resp = enc_int32_add(req->buffer,
								  ENC_INT32_LENGTH,
								  req->buffer + ENC_INT32_LENGTH,
								  ENC_INT32_LENGTH,
								  req->buffer + ENC_INT32_LENGTH + ENC_INT32_LENGTH,
								  ENC_INT32_LENGTH);
		// INFO("add result: %x\n", *((unsigned int *) (req->buffer+ ENC_INT32_LENGTH * 2)));
		break;
	case CMD_INT64_MINUS:
		req->resp = enc_int32_sub(req->buffer,
								  ENC_INT32_LENGTH,
								  req->buffer + ENC_INT32_LENGTH,
								  ENC_INT32_LENGTH,
								  req->buffer + ENC_INT32_LENGTH + ENC_INT32_LENGTH,
								  ENC_INT32_LENGTH);
		break;

	case CMD_INT64_MULT:
		req->resp = enc_int32_mult(req->buffer,
								   ENC_INT32_LENGTH,
								   req->buffer + ENC_INT32_LENGTH,
								   ENC_INT32_LENGTH,
								   req->buffer + ENC_INT32_LENGTH + ENC_INT32_LENGTH,
								   ENC_INT32_LENGTH);
		break;

	case CMD_INT64_DIV:
		req->resp = enc_int32_div(req->buffer,
								  ENC_INT32_LENGTH,
								  req->buffer + ENC_INT32_LENGTH,
								  ENC_INT32_LENGTH,
								  req->buffer + ENC_INT32_LENGTH + ENC_INT32_LENGTH,
								  ENC_INT32_LENGTH);
		break;

	case CMD_INT64_EXP:
		req->resp = enc_int32_pow(req->buffer,
								  ENC_INT32_LENGTH,
								  req->buffer + ENC_INT32_LENGTH,
								  ENC_INT32_LENGTH,
								  req->buffer + ENC_INT32_LENGTH + ENC_INT32_LENGTH,
								  ENC_INT32_LENGTH);
		break;

	case CMD_INT64_MOD:
		req->resp = enc_int32_mod(req->buffer,
								  ENC_INT32_LENGTH,
								  req->buffer + ENC_INT32_LENGTH,
								  ENC_INT32_LENGTH,
								  req->buffer + ENC_INT32_LENGTH + ENC_INT32_LENGTH,
								  ENC_INT32_LENGTH);
		break;
	case CMD_INT64_CMP:
		req->resp = enc_int32_cmp(req->buffer,
								  ENC_INT32_LENGTH,
								  req->buffer + ENC_INT32_LENGTH,
								  ENC_INT32_LENGTH,
								  req->buffer + 2 * ENC_INT32_LENGTH,
								  INT32_LENGTH);
		break;
		// case CMD_INT32_SUM_BULK:
		// 	memcpy(&src_len, req->buffer, INT32_LENGTH);
		// 	req->resp = enc_int32_sum_bulk(
		// 		req->buffer,
		// 		INT32_LENGTH,
		// 		req->buffer + INT32_LENGTH,
		// 		src_len * ENC_INT32_LENGTH,
		// 		req->buffer + (src_len)*ENC_INT32_LENGTH + INT32_LENGTH,
		// 		ENC_INT32_LENGTH);
		// 	break;
	case CMD_FLOAT4_PLUS:
		req->resp = enc_float32_add(req->buffer,
									ENC_FLOAT4_LENGTH,
									req->buffer + ENC_FLOAT4_LENGTH,
									ENC_FLOAT4_LENGTH,
									req->buffer + 2 * ENC_FLOAT4_LENGTH,
									ENC_FLOAT4_LENGTH);
		break;

	case CMD_FLOAT4_MINUS:
		req->resp = enc_float32_sub(req->buffer,
									ENC_FLOAT4_LENGTH,
									req->buffer + ENC_FLOAT4_LENGTH,
									ENC_FLOAT4_LENGTH,
									req->buffer + 2 * ENC_FLOAT4_LENGTH,
									ENC_FLOAT4_LENGTH);
		break;

	case CMD_FLOAT4_MULT:
		req->resp = enc_float32_mult(req->buffer,
									 ENC_FLOAT4_LENGTH,
									 req->buffer + ENC_FLOAT4_LENGTH,
									 ENC_FLOAT4_LENGTH,
									 req->buffer + 2 * ENC_FLOAT4_LENGTH,
									 ENC_FLOAT4_LENGTH);
		break;

	case CMD_FLOAT4_DIV:
		req->resp = enc_float32_div(req->buffer,
									ENC_FLOAT4_LENGTH,
									req->buffer + ENC_FLOAT4_LENGTH,
									ENC_FLOAT4_LENGTH,
									req->buffer + 2 * ENC_FLOAT4_LENGTH,
									ENC_FLOAT4_LENGTH);
		break;

	case CMD_FLOAT4_EXP:
		req->resp = enc_float32_pow(req->buffer,
									ENC_FLOAT4_LENGTH,
									req->buffer + ENC_FLOAT4_LENGTH,
									ENC_FLOAT4_LENGTH,
									req->buffer + 2 * ENC_FLOAT4_LENGTH,
									ENC_FLOAT4_LENGTH);
		break;

	case CMD_FLOAT4_MOD:
		req->resp = enc_float32_mod(req->buffer,
									ENC_FLOAT4_LENGTH,
									req->buffer + ENC_FLOAT4_LENGTH,
									ENC_FLOAT4_LENGTH,
									req->buffer + 2 * ENC_FLOAT4_LENGTH,
									ENC_FLOAT4_LENGTH);
		break;

	case CMD_FLOAT4_CMP:
		req->resp = enc_float32_cmp(req->buffer,
									ENC_FLOAT4_LENGTH,
									req->buffer + ENC_FLOAT4_LENGTH,
									ENC_FLOAT4_LENGTH,
									req->buffer + 2 * ENC_FLOAT4_LENGTH,
									INT32_LENGTH);
		break;

		// case CMD_FLOAT4_SUM_BULK:
		// 	memcpy(&src_len, req->buffer, INT32_LENGTH);
		// 	req->resp = enc_float32_sum_bulk(
		// 		req->buffer,
		// 		INT32_LENGTH,
		// 		req->buffer + INT32_LENGTH,
		// 		src_len * ENC_FLOAT4_LENGTH,
		// 		req->buffer + (src_len)*ENC_FLOAT4_LENGTH + INT32_LENGTH,
		// 		ENC_FLOAT4_LENGTH);
		// 	break;

	case CMD_FLOAT4_ENC:
		req->resp = encrypt_bytes(req->buffer,
								  FLOAT4_LENGTH,
								  req->buffer + FLOAT4_LENGTH,
								  ENC_FLOAT4_LENGTH);
		break;

	case CMD_FLOAT4_DEC:
		req->resp = decrypt_bytes(req->buffer,
								  ENC_FLOAT4_LENGTH,
								  req->buffer + ENC_FLOAT4_LENGTH,
								  FLOAT4_LENGTH);
		break;
		
	default:
		IMSG("Unimplemented Command: 0x%x \n", req->ocall_index);
		break;
	}
#ifdef __TA_PROFILE
	end = read_cntpct();
	duration = (end - start) * 1000000 / freq;
	timer += duration;
	DMSG("duration in us %d, end %d, start %d, freq %d",duration,end,start,freq);
#endif	
	// req->is_done = 1;

	// INFO("is done is set to 1\n");
	/* TODO: re-encrypt result */
	/* TODO: clear footprints? */

	return TEE_SUCCESS;
}

static TEE_Result loadkey(uint32_t param_types,
						  TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INOUT,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE);
	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;


	return TEE_SUCCESS;
}

/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the paramters
 * comes from normal world.
 */
TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
									  uint32_t cmd_id,
									  uint32_t param_types, TEE_Param params[4])
{

	(void)&sess_ctx; /* Unused parameter */

	switch (cmd_id)
	{
	case TA_OPS_CMD_INC_VALUE:
		return inc_value(param_types, params);
	case TA_OPS_CMD_DEC_VALUE:
		return gcm_demo(param_types, params);
	case TA_OPS_CMD_OPS_PROCESS:
		return dec_value(param_types, params);
	case TA_OPS_CMD_LOAD_KEY:
		return loadkey(param_types, params);
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}