#include <enc_int_ops.hpp>
#include <interface.hpp>
#include <request.hpp>
#include <extension_helper.hpp>
#include <extension.hpp>

int enc_int_sum_bulk(size_t bulk_size, EncInt* arg1, EncInt* res)
{
    auto *req = new BulkRequest<EncInt>(CMD_INT_SUM_BULK, bulk_size, arg1, res);
    TEEInvoker *invoker = TEEInvoker::getInstance();
    int resp = invoker->sendRequest(req);
    return resp;
}

int enc_int_ops(int cmd, EncInt* left, EncInt* right, EncInt* res)
{
    auto *req = new CalcRequest<EncInt>(cmd, left, right, res);
    TEEInvoker *invoker = TEEInvoker::getInstance();
    int resp = invoker->sendRequest(req);
    return resp;
}

int enc_int_add(EncInt* left, EncInt* right, EncInt* res)
{
    // ereport(INFO, (errmsg("ADD"))); 
    return enc_int_ops(CMD_INT_PLUS, left, right, res);
}

int enc_int_sub(EncInt* left, EncInt* right, EncInt* res)
{
    // ereport(INFO, (errmsg("SUB"))); 
    int resp = enc_int_ops(CMD_INT_MINUS, left, right, res);
    return resp;
}

int enc_int_mult(EncInt* left, EncInt* right, EncInt* res)
{
    // ereport(INFO, (errmsg("MULT"))); 
    int resp = enc_int_ops(CMD_INT_MULT, left, right, res);
    return resp;
}

int enc_int_div(EncInt* left, EncInt* right, EncInt* res)
{
    // ereport(INFO, (errmsg("DIV"))); 
    int resp = enc_int_ops(CMD_INT_DIV, left, right, res);
    return resp;
}

int enc_int_pow(EncInt* left, EncInt* right, EncInt* res)
{
    int resp = enc_int_ops(CMD_INT_EXP, left, right, res);
    return resp;
}

int enc_int_mod(EncInt* left, EncInt* right, EncInt* res)
{
    int resp = enc_int_ops(CMD_INT_MOD, left, right, res);
    return resp;
}

int enc_int_cmp(EncInt* left, EncInt* right, int* res)
{
    // ereport(INFO, (errmsg("CMP"))); 
    auto *req = new CmpRequest<EncInt, CMD_INT_CMP>(left, right, res);
    TEEInvoker *invoker = TEEInvoker::getInstance();
    int resp = invoker->sendRequest(req);
    return resp;
}

int enc_int_encrypt(int pSrc, EncInt* pDst)
{   
    // ereport(INFO, (errmsg("ENC"))); 
    auto *req = new EncRequest<int, EncInt, CMD_INT_ENC>(&pSrc, pDst);
    TEEInvoker *invoker = TEEInvoker::getInstance();
    int resp = invoker->sendRequest(req);
    return resp;
}

int enc_int_decrypt(EncInt* pSrc, int* pDst)
{
    // ereport(INFO, (errmsg("DEC"))); 
    auto *req = new DecRequest<EncInt,int, CMD_INT_DEC>(pSrc, pDst);
    TEEInvoker *invoker = TEEInvoker::getInstance();
    int resp = invoker->sendRequest(req);
    return resp;
}

int enc_int_get(EncInt* key, EncInt* res)
{
    // Use CalcRequest to get the real EncInt
    // ereport(INFO, (errmsg("GET"))); 
    auto *req = new CalcRequest<EncInt>(CMD_INT_GET_ENC, key, key, res);
    TEEInvoker *invoker = TEEInvoker::getInstance();
    // ereport(INFO, (errmsg("Request sent."))); 
    int resp = invoker->sendRequest(req);
    return resp;
}

int enc_int_dump()
{
    // ereport(INFO, (errmsg("GET")));
    EncInt null_enc_int = EncInt {};
    auto *req = new CalcRequest<EncInt>(CMD_INT_DUMP, &null_enc_int, &null_enc_int, &null_enc_int);
    TEEInvoker *invoker = TEEInvoker::getInstance();
    int resp = invoker->sendRequest(req);
    return resp;
}
