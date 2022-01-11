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
    /* FLOATs */

    case CMD_FLOAT_PLUS:
    case CMD_FLOAT_MINUS:
    case CMD_FLOAT_MULT: 
    case CMD_FLOAT_DIV:
    case CMD_FLOAT_EXP: 
    case CMD_FLOAT_MOD:
        base_req->resp = enc_float32_calc((EncFloatCalcRequestData *)base_req);
        break;
    

    case CMD_FLOAT_CMP:
        base_req->resp = enc_float32_cmp((EncFloatCmpRequestData *)base_req);
        break;
    
        // case CMD_FLOAT_SUM_BULK:
        // 	memcpy(&src_len, req->buffer, INT32_LENGTH);
        // 	req->resp = enc_float32_sum_bulk(
        // 		req->buffer,
        // 		INT32_LENGTH,
        // 		req->buffer + INT32_LENGTH,
        // 		src_len * ENC_FLOAT4_LENGTH,
        // 		req->buffer + (src_len)*ENC_FLOAT4_LENGTH + INT32_LENGTH,
        // 		ENC_FLOAT4_LENGTH);
        // 	break;

    case CMD_FLOAT_ENC:{
        EncFloatEncRequestData *req = (EncFloatEncRequestData *) base_req;
        req->common.resp = encrypt_bytes((uint8_t*) &req->plaintext, sizeof(req->plaintext), 
                                        (uint8_t *) &req->ciphertext, sizeof(req->ciphertext));
        break;
    }
    case CMD_FLOAT_DEC:{
        EncFloatDecRequestData *req = (EncFloatDecRequestData *) base_req;
        req->common.resp = decrypt_bytes((uint8_t *) &req->ciphertext, sizeof(req->ciphertext),
                                        (uint8_t*) &req->plaintext, sizeof(req->plaintext));
        break;
    }




    default:
        break;
    }

    return 0;
}