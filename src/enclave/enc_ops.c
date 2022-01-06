#include <include/enc_ops.h>
#include <request_types.h>
int handle_ops(BaseRequest *base_req)
{
    switch (base_req->reqType)
    {
    case CMD_INT_PLUS:
    {
        EncIntCalcRequestData *req = (EncIntCalcRequestData *) base_req;
        base_req->resp = enc_int32_add(&req->left, sizeof(req->left), &req->right, sizeof(req->right), &req->res,  sizeof(req->res));
        break;
    }
    case CMD_INT_MINUS:
    {
        EncIntCalcRequestData *req = (EncIntCalcRequestData *) base_req;
        base_req->resp = enc_int32_sub(&req->left, sizeof(req->left), &req->right, sizeof(req->right), &req->res,  sizeof(req->res));
        break;
    }
    case CMD_INT_MULT:
    {
        EncIntCalcRequestData *req = (EncIntCalcRequestData *) base_req;
        base_req->resp = enc_int32_mult(&req->left, sizeof(req->left), &req->right, sizeof(req->right), &req->res,  sizeof(req->res));
        break;
    }
    case CMD_INT_DIV:{
        EncIntCalcRequestData *req = (EncIntCalcRequestData *) base_req;
        base_req->resp = enc_int32_div(&req->left, sizeof(req->left), &req->right, sizeof(req->right), &req->res,  sizeof(req->res));
        break;
    }
    case CMD_INT_EXP:{
        EncIntCalcRequestData *req = (EncIntCalcRequestData *) base_req;
        base_req->resp = enc_int32_pow(&req->left, sizeof(req->left), &req->right, sizeof(req->right), &req->res,  sizeof(req->res));
        break;
    }
    case CMD_INT_MOD:{
        EncIntCalcRequestData *req = (EncIntCalcRequestData *) base_req;
        base_req->resp = enc_int32_mod(&req->left, sizeof(req->left), &req->right, sizeof(req->right), &req->res,  sizeof(req->res));
        break;
    }
    case CMD_INT_CMP:{
        EncIntCmpRequestData *req = (EncIntCmpRequestData *) base_req;
        base_req->resp = enc_int32_cmp(&req->left, sizeof(req->left), &req->right, sizeof(req->right), &req->cmp,  sizeof(req->cmp));
        break;
    }
    // case CMD_INT_SUM_BULK:
    //     EncIntBulkRequestData *req = (EncIntBulkRequestData *) base_req;
    //     base_req->resp = enc_int32_sum_bulk(req->bulk_size);
    //     break;

    case CMD_INT_ENC:{
        EncIntEncRequestData *req = (EncIntEncRequestData *) base_req;
        base_req->resp = encrypt_bytes(&req->plaintext,
                                  sizeof(req->plaintext),
                                  &req->ciphertext,
                                  sizeof(req->ciphertext));
        break;
    }
    case CMD_INT_DEC:{
        EncIntDecRequestData *req = (EncIntDecRequestData *) base_req;
        base_req->resp = decrypt_bytes(&req->ciphertext,
                                  sizeof(req->ciphertext),
                                  &req->plaintext,
                                  sizeof(req->plaintext));
        break;
    }
    default:
        break;
    }

    return 0;
}