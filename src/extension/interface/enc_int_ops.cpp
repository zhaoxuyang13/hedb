#include <enc_int_ops.hpp>
#include <interface.hpp>
#include <request.hpp>
#include <extension_helper.hpp>
int enc_int_sum_bulk(size_t bulk_size, EncInt* arg1, EncInt* res)
{
    auto *req = new EncIntBulkRequest(CMD_INT_SUM_BULK, bulk_size, arg1, res);
    TEEInvoker *invoker = TEEInvoker::getInstance();
    int resp = invoker->sendRequest(req);
    return resp;
    
}

int enc_int_ops(int cmd, EncInt* int1, EncInt* int2, EncInt* res)
{
    auto *req = new EncIntCalcRequest(cmd, int1, int2, res);
    TEEInvoker *invoker = TEEInvoker::getInstance();
    int resp = invoker->sendRequest(req);
    return resp;
}

int enc_int_add(EncInt* int1, EncInt* int2, EncInt* res)
{
    return enc_int_ops(CMD_INT_PLUS, int1, int2, res);
}

int enc_int_sub(EncInt* int1, EncInt* int2, EncInt* res)
{
    int resp = enc_int_ops(CMD_INT_MINUS, int1, int2, res);
    return resp;
}

int enc_int_mult(EncInt* int1, EncInt* int2, EncInt* res)
{
    int resp = enc_int_ops(CMD_INT_MULT, int1, int2, res);
    return resp;
}

int enc_int_div(EncInt* int1, EncInt* int2, EncInt* res)
{
    int resp = enc_int_ops(CMD_INT_DIV, int1, int2, res);
    return resp;
}

int enc_int_pow(EncInt* int1, EncInt* int2, EncInt* res)
{
    int resp = enc_int_ops(CMD_INT_EXP, int1, int2, res);
    return resp;
}

int enc_int_mod(EncInt* int1, EncInt* int2, EncInt* res)
{
    int resp = enc_int_ops(CMD_INT_MOD, int1, int2, res);
    return resp;
}

int enc_int_cmp(EncInt* int1, EncInt* int2, int* res)
{
    auto *req = new EncIntCmpRequest(int1, int2, res);
    TEEInvoker *invoker = TEEInvoker::getInstance();
    int resp = invoker->sendRequest(req);
    return resp;
}

int enc_int_encrypt(int pSrc, EncInt* pDst)
{   
    auto *req = new EncIntEncRequest(pSrc, pDst);
    TEEInvoker *invoker = TEEInvoker::getInstance();
    int resp = invoker->sendRequest(req);
    return resp;
}

int enc_int_decrypt(EncInt* pSrc, int* pDst)
{
    auto *req = new EncIntDecRequest(pSrc, pDst);
    TEEInvoker *invoker = TEEInvoker::getInstance();
    int resp = invoker->sendRequest(req);
    return resp;
}
