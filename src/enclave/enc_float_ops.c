#include "enc_float_ops.h"

#if defined(TEE_TZ)
extern double pow(double x, int y);
#endif

int enc_float32_cmp(EncFloatCmpRequestData *req)
{
    float left,right ;
    int resp = 0;
    resp = decrypt_bytes((uint8_t *) &req->left, ENC_FLOAT4_GCD_LENGTH, (uint8_t*) &left, sizeof(float));
    if (resp != 0)
        return resp;

    resp = decrypt_bytes((uint8_t *) &req->right, ENC_FLOAT4_GCD_LENGTH ,(uint8_t*) &right, sizeof(float));
    if (resp != 0)
        return resp;

    req->cmp = (left == right) ? 0 : (left < right) ? -1 : 1;

    return resp;
}




/* comment from PSQL code
 * There isn't any way to check for underflow of addition/subtraction
 * because numbers near the underflow value have already been rounded to
 * the point where we can't detect that the two values were originally
 * different, e.g. on x86, '1e-45'::float4 == '2e-45'::float4 ==
 * 1.4013e-45.
 * we have only 4 bytes for float4 datatype
 * we can check if the out size is less 8^4
 *
 */
int enc_float32_calc(EncFloatCalcRequestData *req)
{
    float left,right,res;
    int resp = 0;
    resp = decrypt_bytes((uint8_t *) &req->left, ENC_FLOAT4_GCD_LENGTH, (uint8_t*) &left, sizeof(left));
    if (resp != 0)
        return resp;

    resp = decrypt_bytes((uint8_t *) &req->right, ENC_FLOAT4_GCD_LENGTH, (uint8_t*) &right, sizeof(right));
    if (resp != 0)
        return resp;
    // printf("clac type %d, %f, %f, ", req->common.reqType, left, right);
    switch (req->common.reqType) /* req->common.op */
    {
    case CMD_FLOAT_PLUS:
        res = left + right;
        break;
    case CMD_FLOAT_MINUS:
        res = left - right;
        break;
    case CMD_FLOAT_MULT: 
        res = left * right;
        break;
    case CMD_FLOAT_DIV:
        res = left / right;
        break;
    case CMD_FLOAT_EXP: 
        res =  pow(left,right);
        break;
    case CMD_FLOAT_MOD:
        res = (int)left % (int)right;
        break;
    default:
        break;
    }

    resp = encrypt_bytes((uint8_t*) &res, sizeof(res),(uint8_t*) &req->res, ENC_FLOAT4_GCD_LENGTH);

    return resp;
}





int enc_float32_bulk(EncFloatBulkRequestData *req)
{
    int bulk_size = req->bulk_size;
    EncFloat *array = req->items;
    float res = 0,tmp = 0;
    int count = 0, resp = 0;
    while (count < bulk_size)
    {
        resp = decrypt_bytes((uint8_t *) &array[count], ENC_FLOAT4_GCD_LENGTH, (uint8_t*) &tmp, sizeof(float));    
        if (resp != 0)
            return resp;

        switch (req->common.reqType)
        {
        case CMD_FLOAT_SUM_BULK:
            res += tmp; 
            break;
        default:
            break;
        }
        count ++;
    }

    resp = encrypt_bytes((uint8_t*) &res, sizeof(float),(uint8_t*) &req->res, ENC_FLOAT4_GCD_LENGTH);
    return resp;
}
