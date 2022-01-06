#include "include/enc_int32_ops.h"


int int2bytearray(int src, uint8_t *pDst, size_t dstLen);
int bytearray2int(uint8_t *pSrc, int *dst, size_t srcLen);

int int2bytearray(int src, uint8_t *pDst, size_t dstLen)
{
    memcpy(pDst, &src, INT32_LENGTH);
    return 0;
}

int bytearray2int(uint8_t *pSrc, int *dst, size_t srcLen)
{
    memcpy(dst, pSrc, INT32_LENGTH);
    return 0;
}

/* Sum of two encrypted by aes_gcm integers
 @input: uint8_t array - encrypted integer1
         size_t - length of encrypted integer1 (SGX_AESGCM_IV_SIZE +
 INT32_LENGTH + SGX_AESGCM_MAC_SIZE = 32) uint8_t array - encrypted integer2
         size_t - length of encrypted integer2 (SGX_AESGCM_IV_SIZE +
 INT32_LENGTH + SGX_AESGCM_MAC_SIZE = 32) uint8_t array - encrypted result
         size_t - length of encrypted result (SGX_AESGCM_IV_SIZE + INT32_LENGTH
 + SGX_AESGCM_MAC_SIZE = 32)
 @return:
    * SGX_error, if there was an error during encryption/decryption
    0, otherwise
*/
int enc_int32_add(uint8_t* int1,
                  size_t int1_len,
                  uint8_t* int2,
                  size_t int2_len,
                  uint8_t* int3,
                  size_t int3_len)
{
    int32_t decint1_int, decint2_int;
    int64_t decint3_int;
    int resp = 0;

    resp = decrypt_bytes(int1,int1_len,&decint1_int, INT32_LENGTH);
    if (resp)
        return resp;

    resp = decrypt_bytes(int2,int2_len,&decint2_int, INT32_LENGTH);
    if (resp)
        return resp;

    // if (bytearray2int(int1, &decint1_int, INT32_LENGTH))
    //     return MEMORY_COPY_ERROR;
    // if (bytearray2int(int2, &decint2_int, INT32_LENGTH))
    //     return MEMORY_COPY_ERROR;
    


	// INFO("%s: int1 %x, int2 %x\n",__func__,(unsigned int)decint1_int, (unsigned int)decint2_int);
    decint3_int = (int64_t)decint1_int + (int64_t)decint2_int;
    
    if ((decint1_int > INT32_MAX || decint1_int < INT32_MIN) || (decint2_int > INT32_MAX || decint2_int < INT32_MIN) || (decint3_int > INT32_MAX || decint3_int < INT32_MIN))
        return OUT_OF_THE_RANGE_ERROR;

    if (int2bytearray((int32_t)decint3_int, int3, INT32_LENGTH))
        return MEMORY_COPY_ERROR;

    resp = encrypt_bytes((int8_t *)&decint3_int, INT32_LENGTH, int3, int3_len);

    memset(&decint1_int, 0, sizeof(int32_t));
    memset(&decint2_int, 0, sizeof(int32_t));
    memset(&decint3_int, 0, sizeof(int64_t));

    return resp;
}

int enc_int32_sub(uint8_t* int1,
                  size_t int1_len,
                  uint8_t* int2,
                  size_t int2_len,
                  uint8_t* int3,
                  size_t int3_len)
{
    int32_t decint1_int, decint2_int;
    int64_t decint3_int;
    int resp;

    resp = decrypt_bytes(int1,int1_len,&decint1_int, INT32_LENGTH);
    if (resp)
        return resp;
    resp = decrypt_bytes(int2,int2_len,&decint2_int, INT32_LENGTH);
    if (resp)
        return resp;

    decint3_int = (int64_t)decint1_int - (int64_t)decint2_int;

    if ((decint1_int > INT32_MAX || decint1_int < INT32_MIN) || (decint2_int > INT32_MAX || decint2_int < INT32_MIN) || (decint3_int > INT32_MAX || decint3_int < INT32_MIN))
        return OUT_OF_THE_RANGE_ERROR;

    resp = encrypt_bytes((int8_t *)&decint3_int, INT32_LENGTH, int3, int3_len);

    memset(&decint1_int, 0, sizeof(int32_t));
    memset(&decint2_int, 0, sizeof(int32_t));
    memset(&decint3_int, 0, sizeof(int64_t));

    return resp;
}
int enc_int32_mult(uint8_t* int1,
                   size_t int1_len,
                   uint8_t* int2,
                   size_t int2_len,
                   uint8_t* int3,
                   size_t int3_len)
{
    int32_t decint1_int, decint2_int;
    int64_t decint3_int;
    int resp;

    resp = decrypt_bytes(int1,int1_len,&decint1_int, INT32_LENGTH);
    if (resp)
        return resp;
    resp = decrypt_bytes(int2,int2_len,&decint2_int, INT32_LENGTH);
    if (resp)
        return resp;

    decint3_int = decint1_int * decint2_int;

    if ((decint1_int > INT32_MAX || decint1_int < INT32_MIN) || (decint2_int > INT32_MAX || decint2_int < INT32_MIN) || (decint3_int > INT32_MAX || decint3_int < INT32_MIN))
        return OUT_OF_THE_RANGE_ERROR;

    resp = encrypt_bytes((int8_t *)&decint3_int, INT32_LENGTH, int3, int3_len);

    memset(&decint1_int, 0, sizeof(int32_t));
    memset(&decint2_int, 0, sizeof(int32_t));
    memset(&decint3_int, 0, sizeof(int64_t));

    return resp;
}
int enc_int32_mod(uint8_t* int1,
                  size_t int1_len,
                  uint8_t* int2,
                  size_t int2_len,
                  uint8_t* int3,
                  size_t int3_len)
{
    int32_t decint1_int, decint2_int;
    int64_t decint3_int;
    int resp;

    resp = decrypt_bytes(int1,int1_len,&decint1_int, INT32_LENGTH);
    if (resp)
        return resp;
    resp = decrypt_bytes(int2,int2_len,&decint2_int, INT32_LENGTH);
    if (resp)
        return resp;

    if (decint2_int == 0)
        return ARITHMETIC_ERROR;

    decint3_int = decint1_int % decint2_int;

    if ((decint1_int > INT32_MAX || decint1_int < INT32_MIN) || (decint2_int > INT32_MAX || decint2_int < INT32_MIN) || (decint3_int > INT32_MAX || decint3_int < INT32_MIN))
        return OUT_OF_THE_RANGE_ERROR;

   resp = encrypt_bytes((int8_t *)&decint3_int, INT32_LENGTH, int3, int3_len);

    memset(&decint1_int, 0, sizeof(int32_t));
    memset(&decint2_int, 0, sizeof(int32_t));
    memset(&decint3_int, 0, sizeof(int64_t));

    return resp;
}
// double pow (double x, int y)
// {
//     double temp;
//     if (y == 0)
//     return 1;
//     temp = pow (x, y / 2);
//     if ((y % 2) == 0) {
//         return temp * temp;
//     } else {
//         if (y > 0)
//             return x * temp * temp;
//         else
//             return (temp * temp) / x;
//     }
// }
int enc_int32_pow(uint8_t* int1,
                  size_t int1_len,
                  uint8_t* int2,
                  size_t int2_len,
                  uint8_t* int3,
                  size_t int3_len)
{
    int32_t decint1_int, decint2_int;
    int64_t decint3_int;
    int resp;

    resp = decrypt_bytes(int1,int1_len,&decint1_int, INT32_LENGTH);
    if (resp)
        return resp;
    resp = decrypt_bytes(int2,int2_len,&decint2_int, INT32_LENGTH);
    if (resp)
        return resp;


    decint3_int = (int64_t)pow((double)decint1_int, decint2_int);


    if ((decint1_int > INT32_MAX || decint1_int < INT32_MIN) || (decint2_int > INT32_MAX || decint2_int < INT32_MIN) || (decint3_int > INT32_MAX || decint3_int < INT32_MIN))
        return OUT_OF_THE_RANGE_ERROR;

   resp = encrypt_bytes((int8_t *)&decint3_int, INT32_LENGTH, int3, int3_len);

    memset(&decint1_int, 0, sizeof(int32_t));
    memset(&decint2_int, 0, sizeof(int32_t));
    memset(&decint3_int, 0, sizeof(int64_t));

    return resp;
}
int enc_int32_div(uint8_t* int1,
                  size_t int1_len,
                  uint8_t* int2,
                  size_t int2_len,
                  uint8_t* int3,
                  size_t int3_len)
{
    int32_t decint1_int, decint2_int;
    int64_t decint3_int;
    int resp;

    resp = decrypt_bytes(int1,int1_len,&decint1_int, INT32_LENGTH);
    if (resp)
        return resp;
    resp = decrypt_bytes(int2,int2_len,&decint2_int, INT32_LENGTH);
    if (resp)
        return resp;

    if (decint2_int == 0)
        return ARITHMETIC_ERROR;

    decint3_int = decint1_int / decint2_int;

    if ((decint1_int > INT32_MAX || decint1_int < INT32_MIN) || (decint2_int > INT32_MAX || decint2_int < INT32_MIN) || (decint3_int > INT32_MAX || decint3_int < INT32_MIN))
        return OUT_OF_THE_RANGE_ERROR;

   resp = encrypt_bytes((int8_t *)&decint3_int, INT32_LENGTH, int3, int3_len);

    memset(&decint1_int, 0, sizeof(int32_t));
    memset(&decint2_int, 0, sizeof(int32_t));
    memset(&decint3_int, 0, sizeof(int64_t));

    return resp;
}

int enc_int32_cmp(uint8_t* int1,
                  size_t int1_len,
                  uint8_t* int2,
                  size_t int2_len,
                  uint8_t* result,
                  size_t res_len)
{
    int32_t decint1_int, decint2_int;
	int32_t cmp;
    int resp = 0;

    resp = decrypt_bytes(int1,int1_len,&decint1_int, INT32_LENGTH);
    if (resp)
        return resp;
    resp = decrypt_bytes(int2,int2_len,&decint2_int, INT32_LENGTH);
    if (resp)
        return resp;



    cmp = (decint1_int == decint2_int)
              ? 0
              : (decint1_int < decint2_int) ? -1 : 1;

    // DMSG("%d, %d, cmp: %d\n",decint1_int, decint2_int, cmp);
    memcpy(result, &cmp, res_len);
    memset(&decint1_int, 0, sizeof(int32_t));
    memset(&decint2_int, 0, sizeof(int32_t));

    return resp;
}
/* Sum several aes_gcm-encrypted integers
 @input: uint8_t array - encrypted integer1
         size_t - length of encrypted integer1 (SGX_AESGCM_IV_SIZE +
 INT32_LENGTH + SGX_AESGCM_MAC_SIZE = 32)
         uint8_t array - encrypted integer2
         size_t - length of encrypted integer2 (SGX_AESGCM_IV_SIZE +
 INT32_LENGTH + SGX_AESGCM_MAC_SIZE = 32)
         uint8_t array - encrypted result
         size_t - length of encrypted result (SGX_AESGCM_IV_SIZE + INT32_LENGTH
 + SGX_AESGCM_MAC_SIZE = 32)
 @return:
    * SGX_error, if there was an error during encryption/decryption
    0, otherwise
*/
int enc_int32_sum_bulk(uint8_t* arg1,
                       size_t arg1_len,
                       uint8_t* arg2,
                       size_t arg2_len,
                       uint8_t* int3,
                       size_t int3_len)
{
    int32_t decint1_int, decint2_int;
    int64_t decint3_int;
    int32_t bulk_size = 0;
    int resp, counter = 0;
    int32_t current_position = 0;
    uint8_t* dec_int1_v = (uint8_t*)malloc(INT32_LENGTH);
    uint8_t* dec_int2_v = (uint8_t*)malloc(INT32_LENGTH);
    uint8_t* dec_int3_v = (uint8_t*)malloc(INT32_LENGTH);

    if (!dec_int1_v || !dec_int2_v || !dec_int3_v)
    {
        return MEMORY_ALLOCATION_ERROR;
        ;
    }

    if (bytearray2int(arg1, bulk_size, INT32_LENGTH))
        return MEMORY_COPY_ERROR;
    decint3_int = 0;

    while (counter < bulk_size)
    {
        resp = decrypt_bytes(
            arg2 + current_position, ENC_INT32_LENGTH, dec_int2_v, INT32_LENGTH);
        if (resp != 0)
            return resp;
        current_position += ENC_INT32_LENGTH;

        if (bytearray2int(dec_int2_v, decint2_int, INT32_LENGTH))
            return MEMORY_COPY_ERROR;

        decint3_int += (int64_t)decint2_int;
        counter++;
    }

    if ((decint1_int > INT32_MAX || decint1_int < INT32_MIN) || (decint3_int > INT32_MAX || decint3_int < INT32_MIN))
        return OUT_OF_THE_RANGE_ERROR;

    if (int2bytearray((int32_t)decint3_int, dec_int3_v, INT32_LENGTH))
        return MEMORY_COPY_ERROR;

    resp = encrypt_bytes(dec_int3_v, INT32_LENGTH, int3, int3_len);

    memset(dec_int1_v, 0, INT32_LENGTH);
    memset(dec_int2_v, 0, INT32_LENGTH);
    memset(dec_int3_v, 0, INT32_LENGTH);

    memset(&decint1_int, 0, sizeof(int32_t));
    memset(&decint2_int, 0, sizeof(int32_t));
    memset(&decint3_int, 0, sizeof(int64_t));

    free(dec_int1_v);
    free(dec_int2_v);
    free(dec_int3_v);
    return resp;
}
