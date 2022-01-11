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

/* DataType should be one of CMP DATA */

template <typename EncType>
class CmpRequest : public Request {
public:
    DEFINE_ENCTYPE_CMP_ReqData(EncType);
    CmpRequest(EncType *left, EncType *right, int *cmp) : left(left), right(right), cmp(cmp) {}

    EncType *left;
    EncType *right;
    int *cmp;

    void serializeTo(void *buffer) const override  {
        auto *req = (EncTypeCmpRequestData *) buffer;
        req->common.reqType = CMD_INT_CMP;
        req->left = *left;
        req->right = *right;
    }

    void copyResultFrom(void *buffer)const override  {
        auto *req = (EncTypeCmpRequestData *) buffer;
        *cmp = req->cmp;
    }
};


template <typename EncType>
class CalcRequest : public Request {
public:
    DEFINE_ENCTYPE_CALC_ReqData(EncType);
    int op;
    EncType *left;
    EncType *right;
    EncType *res;

    CalcRequest(int op, EncType *left, EncType *right, EncType *res) : op(op), left(left), right(right), res(res) {}

    void serializeTo(void *buffer) const override {
        auto *req = (EncTypeCalcRequestData *) buffer;
        req->common.reqType = op;
        req->op = op;
        req->left = *left;
        req->right = *right;
    }

    void copyResultFrom(void *buffer)const override  {
        auto *req = (EncTypeCalcRequestData *) buffer;
        *res = req->res;
    }
};

template<typename EncType>
class BulkRequest : public Request {
public:
    DEFINE_ENCTYPE_BULK_ReqData(EncType)
    int bulk_type;
    int bulk_size;
    EncType *items; //begin of items
    EncType *res;   // Maybe EncFloat in average, use union that-wise

    BulkRequest(int bulkType, int bulkSize, EncType *items, EncType *res) : bulk_type(bulkType),
                                                                                bulk_size(bulkSize), items(items),
                                                                                res(res) {}

    void serializeTo(void *buffer) const override  {
        auto *req = (EncTypeBulkRequestData *)buffer;
        req->common.reqType = bulk_type;
        req->bulk_size = bulk_size;
        memcpy(req->items, items, sizeof(EncType) * bulk_size);
    }

    void copyResultFrom(void *buffer)const override  {
        auto *req = (EncTypeBulkRequestData *)buffer;
        *res = req->res;
    }
};

/* TODO: define = operator for encstr type, and other types
         change encstr type to {size, char[]}
*/
template<typename PlainType, typename EncType>
class EncRequest : public Request {
public:
    DEFINE_ENCTYPE_ENC_ReqData(EncType,PlainType);

    PlainType *plaintext;
    EncType *res;

    EncRequest(PlainType *plaintext, EncType *res) : plaintext(plaintext), res(res) {}

    void serializeTo(void *buffer) const override {
        auto *req = (EncTypeEncRequestData *) buffer;
        req->common.reqType = CMD_INT_ENC;
        req->plaintext = *plaintext;
    }

    void copyResultFrom(void *buffer)const override {
        auto *req = (EncTypeEncRequestData *) buffer;
        *res = req->ciphertext;
    }
};

template<typename EncType,typename PlainType>
class DecRequest : public Request {
public:
    DEFINE_ENCTYPE_DEC_ReqData(EncType, PlainType);
    EncType *ciphertext;
    PlainType *res;
    
    DecRequest(EncType *ciphertext, PlainType *res) : ciphertext(ciphertext), res(res) {}

    void serializeTo(void *buffer) const override{
        auto *req = (EncTypeDecRequestData *) buffer;
        req->common.reqType = CMD_INT_DEC;
        req->ciphertext = *ciphertext;
    }

    void copyResultFrom(void *buffer)const override {
        auto *req = (EncTypeDecRequestData *) buffer;
        *res = req->plaintext;
    }
};