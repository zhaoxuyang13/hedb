#include "enc_timestamp_ops.h"
#include "kv.h"

int enc_timestamp_cmp(EncTimestampCmpRequestData *req)
{
    timestamp_comp_count++;
    int resp = 0;

    TIMESTAMP left, right;

    resp = decrypt_bytes((uint8_t *) &req->left, sizeof(req->left),(uint8_t*) &left, sizeof(left));
    if (resp != 0)
        return resp;

    resp = decrypt_bytes((uint8_t *) &req->right, sizeof(req->right),(uint8_t*) &right, sizeof(right));
    if (resp != 0)
        return resp;

    req->cmp = (left == right) ? 0 : (left < right) ? -1 : 1;

    return resp;
}

// int enc_timestamp_extract_year(EncTimestamp *in, EncInt *out)
// {

// }
