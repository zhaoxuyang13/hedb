#include <enc_int_ops.hpp>
#include <interface.hpp>
#include <request.hpp>
#include <extension_helper.hpp>

int enc_int_sum_bulk(size_t bulk_size, EncInt* arg1, EncInt* res)
{
    int sum = 0, num = 0;
    uint64_t mlen, clen;
    Cryptor *cptr = Cryptor::getInstance();
    for (size_t i = 0; i < bulk_size; ++i) {
        cptr->decrypt((uint8_t *) &num, &mlen, (uint8_t *)(&arg1[i]), sizeof(int) + 16U);
        sum += num;
    }
    cptr->encrypt((uint8_t *)&sum, sizeof(int), (uint8_t *)res, &clen);
    return 0;
}

int enc_int_ops(int cmd, EncInt* left, EncInt* right, EncInt* res)
{
    int l, r;
    uint64_t mlen, clen;
    Cryptor *cptr = Cryptor::getInstance();
    cptr->decrypt((uint8_t *) &l, &mlen, (uint8_t *)left, sizeof(int) + 16U);
    cptr->decrypt((uint8_t *) &r, &mlen, (uint8_t *)right, sizeof(int) + 16U);
    int result;
    switch (cmd) /* req->common.op */
    {
    case CMD_INT_PLUS:
        result = l + r;
        break;
    case CMD_INT_MINUS:
        result = l - r;
        break;
    case CMD_INT_MULT: 
        result = l * r;
        break;
    case CMD_INT_DIV:
        result = l / r;
        break;
    case CMD_INT_EXP: 
        result = pow(l,r);
        break;
    case CMD_INT_MOD:
        result = (int)l % (int)r;
        break;
    default:
        break;
    }
    cptr->encrypt((uint8_t *)&result, sizeof(int), (uint8_t *)res, &clen);
    return 0;
}

int enc_int_add(EncInt* left, EncInt* right, EncInt* res)
{
    return enc_int_ops(CMD_INT_PLUS, left, right, res);
}

int enc_int_sub(EncInt* left, EncInt* right, EncInt* res)
{
    int resp = enc_int_ops(CMD_INT_MINUS, left, right, res);
    return resp;
}

int enc_int_mult(EncInt* left, EncInt* right, EncInt* res)
{
    int resp = enc_int_ops(CMD_INT_MULT, left, right, res);
    return resp;
}

int enc_int_div(EncInt* left, EncInt* right, EncInt* res)
{
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
    int l, r;
    uint64_t mlen;
    Cryptor *cptr = Cryptor::getInstance();
    cptr->decrypt((uint8_t *) &l, &mlen, (uint8_t *)left, sizeof(int) + 16U);
    cptr->decrypt((uint8_t *) &r, &mlen, (uint8_t *)right, sizeof(int) + 16U);
    *res = (l == r) ? 0 : (l < r) ? -1 : 1;
    return 0;
}

int enc_int_encrypt(int pSrc, EncInt* pDst)
{   
    uint64_t clen;
    Cryptor *cptr = Cryptor::getInstance();
    cptr->encrypt((uint8_t *)&pSrc, sizeof(int), (uint8_t *)pDst, &clen);
    return 0;
}

int enc_int_decrypt(EncInt* pSrc, int* pDst)
{
    uint64_t mlen;
    Cryptor *cptr = Cryptor::getInstance();
    cptr->decrypt((uint8_t *)pDst, &mlen, (uint8_t *)pSrc, sizeof(int) + 16U);
    return 0;
}
