#include "enc_int_ops.h"
#include "kv.h"

#if defined(TEE_TZ)

double pow (double x, int y)
{
    double temp;
    if (y == 0)
    return 1;
    temp = pow (x, y / 2);
    if ((y % 2) == 0) {
        return temp * temp;
    } else {
        if (y > 0)
            return x * temp * temp;
        else
            return (temp * temp) / x;
    }
}
#endif

int enc_int32_calc(EncIntCalcRequestData *req){
    int left,right,res;
    int resp = 0;

    if (req->common.reqType == CMD_INT_PRINT_STATS) {
        printf("int_comp_count: %d", int_comp_count);
        printf("int_calc_count: %d", int_calc_count);
        printf("int_bulk_count: %d", int_bulk_count);
        printf("int_bulk_dec_count: %d", int_bulk_dec_count);
        printf("float_comp_count: %d", float_comp_count);
        printf("float_calc_count: %d", float_calc_count);
        printf("float_bulk_count: %d", float_bulk_count);
        printf("float_bulk_dec_count: %d", float_bulk_dec_count);
        printf("timestamp_comp_count: %d", timestamp_comp_count);
        printf("text_comp_count: %d", text_comp_count);
        printf("text_like_count: %d", text_like_count);
        printf("text_cat_count: %d", text_cat_count);
        printf("text_substr_count: %d", text_substr_count);
        // uint64_t begin = rdtsc();
        // uint8_t src[1052] = {0}, dest[1024] = {0};
        // for (int i = 0; i < 10000; ++i) {
        //     decrypt_bytes((uint8_t *) src, sizeof(src), (uint8_t *) dest, sizeof(res));
        // }
        // uint64_t end = rdtsc();
        // printf("Time: %llu", end-begin);

        for (int i = 0; i < 50000; ++i) {
            int_map_insert(int_buf_p, i);
        }
        EncInt key = int_map_insert(int_buf_p, 50000);
        for (int i = 50001; i < 200000; ++i) {
            int_map_insert(int_buf_p, i);
        }
        uint64_t begin = rdtsc();
        int_map_find(int_buf_p, key);
        uint64_t end = rdtsc();
        printf("Time: %llu", end-begin);
        return 0;
    }
    // if request to get real EncInt, go to int_map and find int value, then encrypt
    // if (req->common.reqType == CMD_INT_GET_ENC) {
    //     res = int_map_find(int_buf_p, req->left);
    //     resp = encrypt_bytes((uint8_t*) &res, sizeof(res),(uint8_t*) &req->res, sizeof(req->res));
    //     return resp;
    // }
    // if left and right stores the key, go to int map and find int value instead of decrypt

    // if (memcmp(req->left.iv, &IV_GLOBAL_ZERO, IV_SIZE) == 0) {
    //     left = int_map_find(int_buf_p, req->left);
    // } else {
    resp = decrypt_bytes((uint8_t *) &req->left, sizeof(req->left), (uint8_t*) &left, sizeof(left));
    if (resp != 0) {
        return resp;
    }
    // }
    // if (memcmp(req->right.iv, &IV_GLOBAL_ZERO, IV_SIZE) == 0) {
    //     right = int_map_find(int_buf_p, req->right);
    // } else {
        
    resp = decrypt_bytes((uint8_t *) &req->right, sizeof(req->right), (uint8_t*) &right, sizeof(right));
    if (resp != 0) {
        return resp;
    }

    // }
    // printf("clac type %d, %d, %d, ", req->common.reqType, left, right);
    switch (req->common.reqType) /* req->common.op */
    {
    case CMD_INT_PLUS:
        res = left + right;
        break;
    case CMD_INT_MINUS:
        res = left - right;
        break;
    case CMD_INT_MULT: 
        res = left * right;
        break;
    case CMD_INT_DIV:
        res = left / right;
        break;
    case CMD_INT_EXP: 
        res =  pow(left,right);
        break;
    case CMD_INT_MOD:
        res = (int)left % (int)right;
        break;
    default:
        break;
    }

    // req->res = int_map_insert(int_buf_p, res);

    // return 0;

    resp = encrypt_bytes((uint8_t*) &res, sizeof(res),(uint8_t*) &req->res, sizeof(req->res));

    int_calc_count++;

    return resp;
}

int enc_int32_cmp(EncIntCmpRequestData *req){
    int left,right ;
    int resp = 0;
    resp = decrypt_bytes((uint8_t *) &req->left, sizeof(req->left),(uint8_t*) &left, sizeof(left));
    if (resp != 0)
        return resp;

    resp = decrypt_bytes((uint8_t *) &req->right, sizeof(req->right),(uint8_t*) &right, sizeof(right));
    if (resp != 0)
        return resp;

    req->cmp = (left == right) ? 0 : (left < right) ? -1 : 1;

    int_comp_count++;

    return resp;
}

int enc_int32_bulk(EncIntBulkRequestData *req){
    int bulk_size = req->bulk_size;
    EncInt *array = req->items;
    int res = 0,tmp = 0;
    int count = 0, resp = 0;
    while (count < bulk_size)
    {
        resp = decrypt_bytes((uint8_t *) &array[count], sizeof(EncInt), (uint8_t*) &tmp, sizeof(tmp));    
        if (resp != 0)
            return resp;

        switch (req->common.reqType)
        {
        case CMD_INT_SUM_BULK:
            res += tmp; 
            break;
        default:
            break;
        }
        count ++;
    }

    resp = encrypt_bytes((uint8_t*) &res, sizeof(res),(uint8_t*) &req->res, sizeof(req->res));

    int_bulk_count++;

    return resp;
}

