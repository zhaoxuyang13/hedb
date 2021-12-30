#pragma once
extern "C"
{
#include <defs.h>
#include <enc_types.h>
#include <request_types.h>
}

class Request {
public:
    virtual void serializeTo(void *buffer) const = 0;

    virtual void copyResultFrom(void *buffer) const = 0;

    virtual inline int size() { return 0; };

private:
};
//typedef enum RequestType {
//
//}ReqType;
class EncIntCmpRequest : public Request {
public:
    EncIntCmpRequest(EncInt *left, EncInt *right, int *cmp) : left(left), right(right), cmp(cmp) {}

    EncInt *left;
    EncInt *right;
    int *cmp;


    void serializeTo(void *buffer) const override  {
        auto *req = (EncIntCmpRequestData *) buffer;
        req->common.reqType = CMD_INT_CMP;
        req->left = *left;
        req->right = *right;
    }

    void copyResultFrom(void *buffer)const override  {
        auto *req = (EncIntCmpRequestData *) buffer;
        *cmp = req->cmp;
    }
};

class EncIntCalcRequest : public Request {
public:
    int op;
    EncInt *left;
    EncInt *right;
    EncInt *res;

    EncIntCalcRequest(int op, EncInt *left, EncInt *right, EncInt *res) : op(op), left(left), right(right), res(res) {}

    void serializeTo(void *buffer) const override {
        auto *req = (EncIntCalcRequestData *) buffer;
        req->common.reqType = op;
        req->op = op;
        req->left = *left;
        req->right = *right;
    }

    void copyResultFrom(void *buffer)const override  {
        auto *req = (EncIntCalcRequestData *) buffer;
        *res = req->res;
    }
};

class EncIntBulkRequest : public Request {
public:
    int bulk_type;
    int bulk_size;
    EncInt *items; //begin of items
    EncInt *res;   // Maybe EncFloat in average, use union that-wise

    EncIntBulkRequest(int bulkType, int bulkSize, EncInt *items, EncInt *res) : bulk_type(bulkType),
                                                                                bulk_size(bulkSize), items(items),
                                                                                res(res) {}


    void serializeTo(void *buffer) const override  {
        auto *req = (EncIntBulkRequestData *)buffer;
        req->common.reqType = bulk_type;
        req->bulk_size = bulk_size;
        memcpy(req->items, items, sizeof(EncInt) * bulk_size);
    }

    void copyResultFrom(void *buffer)const override  {
        auto *req = (EncIntBulkRequestData *)buffer;
        *res = req->res;
    }
};

class EncIntEncRequest : public Request {
public:
    int plaintext;

    EncIntEncRequest(int plaintext, EncInt *res) : plaintext(plaintext), res(res) {}

    EncInt *res;

    void serializeTo(void *buffer) const override {
        auto *req = (EncIntEncRequestData *) buffer;
        req->common.reqType = CMD_INT_ENC;
        req->plaintext = plaintext;
    }

    void copyResultFrom(void *buffer)const override {
        auto *req = (EncIntEncRequestData *) buffer;
        *res = req->ciphertext;
    }
};

class EncIntDecRequest : public Request {
public:
    EncInt *ciphertext;
    int *res;
    
    EncIntDecRequest(EncInt *ciphertext, int *res) : ciphertext(ciphertext), res(res) {}

    void serializeTo(void *buffer) const override{
        auto *req = (EncIntDecRequestData *) buffer;
        req->common.reqType = CMD_INT_DEC;
        req->ciphertext = *ciphertext;
    }

    void copyResultFrom(void *buffer)const override {
        auto *req = (EncIntDecRequestData *) buffer;
        *res = req->plaintext;
    }
};