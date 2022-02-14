#include <enc_ops.h>
#include <request_types.h>
int handle_ops(BaseRequest *base_req)
{
    switch (base_req->reqType)
    {
    case CMD_INT_PLUS:
    case CMD_INT_MINUS:
    case CMD_INT_MULT:
    case CMD_INT_DIV:
    case CMD_INT_EXP:
    case CMD_INT_MOD:
        base_req->resp = enc_int32_calc((EncIntCalcRequestData *)base_req);
        break; 
    case CMD_INT_CMP:
        base_req->resp = enc_int32_cmp((EncIntCmpRequestData *)base_req);
        break;
    case CMD_INT_SUM_BULK:
        base_req->resp = enc_int32_bulk((EncIntBulkRequestData *)base_req);
        break;

    case CMD_INT_ENC:{
        EncIntEncRequestData *req = (EncIntEncRequestData *) base_req;
        base_req->resp = encrypt_bytes((uint8_t*) &req->plaintext, sizeof(req->plaintext),
                                  (uint8_t*) &req->ciphertext, sizeof(req->ciphertext));
        break;
    }
    case CMD_INT_DEC:{
        EncIntDecRequestData *req = (EncIntDecRequestData *) base_req;
        base_req->resp = decrypt_bytes((uint8_t*)&req->ciphertext, sizeof(req->ciphertext),
                                  (uint8_t*)&req->plaintext, sizeof(req->plaintext));
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
    
    case CMD_FLOAT_SUM_BULK:
        base_req->resp = enc_float32_bulk((EncFloatBulkRequestData *) base_req);
        break;

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