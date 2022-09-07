#include "enc_int_ops.h"

// extern int decrypt_status; 
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
/* this load barrier is only for arm */
#ifdef __aarch64__
	#define LOAD_BARRIER asm volatile("dsb ld" ::: "memory")
	#define STORE_BARRIER asm volatile("dsb st" ::: "memory")
#elif __x86_64
	#define LOAD_BARRIER ;
	#define STORE_BARRIER ;
#endif

int enc_int32_calc(EncIntCalcRequestData *req){

    int left,right,res;
    int resp = 0;
    // printf("call add\n");
    resp = decrypt_bytes_para((uint8_t *) &req->left, sizeof(req->left), (uint8_t*) &left, sizeof(left));

    if (resp != 0)
        return resp;
    // printf("req sent\n");

    resp = decrypt_bytes((uint8_t *) &req->right, sizeof(req->right),(uint8_t*) &right, sizeof(right));
    if (resp != 0)
        return resp;
    
    decrypt_wait((uint8_t*) &left, sizeof(left));

    // printf("clac type %d, %f, %f, ", req->common.reqType, left, right);
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
    // printf("%d, %d, %d, %d\n",req->common.reqType, left, right, res);
    resp = encrypt_bytes((uint8_t*) &res, sizeof(res),(uint8_t*) &req->res, sizeof(req->res));
    return resp;
}

int enc_int32_cmp(EncIntCmpRequestData *req){
    int left,right ;
    int resp = 0;
    resp = decrypt_bytes_para((uint8_t *) &req->left, sizeof(req->left),(uint8_t*) &left, sizeof(left));
    if (resp != 0)
        return resp;

    resp = decrypt_bytes((uint8_t *) &req->right, sizeof(req->right),(uint8_t*) &right, sizeof(right));
    if (resp != 0)
        return resp;
    decrypt_wait((uint8_t*) &left, sizeof(left));

    req->cmp = (left == right) ? 0 : (left < right) ? -1 : 1;
    // printf("%d, %d, %d, %d\n",req->common.reqType, left, right, req->cmp);

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
    return resp;
}

