#pragma once
extern "C"
{
#include <defs.h>
#include <enc_types.h>
#include <request_types.h>
}

#include <extension_helper.hpp>



#define IS_ENCSTR(a) (sizeof(*a) == sizeof(EncStr))
#define ENCSTR_LEN(a) (((EncStr*)a)->len)
#define ENCSTR_DATA(a) (&((EncStr*)a)->enc_cstr)
#define COPY_ENC(to,from) { \
    ENCSTR_LEN(to) = ENCSTR_LEN(from);                        \
    memcpy(ENCSTR_DATA(to),ENCSTR_DATA(from), ENCSTR_LEN(to)); }

#define IS_STR(a) (sizeof(*a) == sizeof(Str))
#define STR_LEN(a) (((Str*)a)->len)
#define STR_DATA(a) (((Str*)a)->data)
#define COPY_PLAIN(to,from) { \
    STR_LEN(to) = STR_LEN(from);                        \
    memcpy(STR_DATA(to),STR_DATA(from), STR_LEN(to)); }

#define COPY(to, from) { \
    if      (IS_STR(from)) {   COPY_PLAIN(to,from); }\
    else if (IS_ENCSTR(from)) {COPY_ENC(to, from);} \
    else    *to = *from; }

class Request {
public:
    virtual void serializeTo(void *buffer) const = 0;

    virtual void copyResultFrom(void *buffer) const = 0;

    virtual inline int size() { return 0; };

private:
};

/* DataType should be one of CMP DATA */

template <typename EncType, int reqType>
class CmpRequest : public Request {
public:
    DEFINE_ENCTYPE_CMP_ReqData(EncType);
    CmpRequest(EncType *left, EncType *right, int *cmp) : left(left), right(right), cmp(cmp) {}

    EncType *left;
    EncType *right;
    int *cmp;

    void serializeTo(void *buffer) const override  {
        auto *req = (EncTypeCmpRequestData *) buffer;
        req->common.reqType = reqType;
        COPY(&req->left, left);
        COPY(&req->right, right);
        // req->left = *left;
        // req->right = *right;
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
        COPY(&req->left, left);
        COPY(&req->right, right);
    }

    void copyResultFrom(void *buffer)const override  {
        auto *req = (EncTypeCalcRequestData *) buffer;
        COPY(res, &req->res);
        // *res = req->res;
    }
};
// template<>  // specialization
// void CalcRequest<EncStr>::copyResultFrom(void *buffer) const override1{
//     auto *req = (EncTypeCalcRequestData *) buffer;
//     // EncStr *estr = (EncStr *) res;
//     *res = req->res;
// }



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
template<typename PlainType, typename EncType, int reqType>
class EncRequest : public Request {
public:
    DEFINE_ENCTYPE_ENC_ReqData(EncType,PlainType);

    PlainType *plaintext;
    EncType *res;

    EncRequest(PlainType *plaintext, EncType *res) : plaintext(plaintext), res(res) {}

    void serializeTo(void *buffer) const override {
        auto *req = (EncTypeEncRequestData *) buffer;
        req->common.reqType = reqType;
        // req->plaintext = *plaintext;
        COPY(&req->plaintext, plaintext);

        // if(reqType == CMD_STRING_ENC){
        //     char ch[100];
        //     sprintf(ch, "after serialize %d", ((Str *) plaintext)->len);
        //     print_info(ch);
        // }
    }

    void copyResultFrom(void *buffer)const override {
        // if(reqType == CMD_STRING_ENC){
        //     char ch[100];
        //     sprintf(ch, "before copy result %d", ((Str *) plaintext)->len);
        //     print_info(ch);
        // }
        auto *req = (EncTypeEncRequestData *) buffer;
        COPY(res, &req->ciphertext);
        // *res = req->ciphertext;
        // if(reqType == CMD_STRING_ENC){
        //     char ch[100];
        //     sprintf(ch, "after copy result %d", ((Str *) plaintext)->len);
        //     print_info(ch);
        // }
    }
};

template<typename EncType,typename PlainType,int reqType>
class DecRequest : public Request {
public:
    DEFINE_ENCTYPE_DEC_ReqData(EncType, PlainType);
    EncType *ciphertext;
    PlainType *res;
    
    DecRequest(EncType *ciphertext, PlainType *res) : ciphertext(ciphertext), res(res) {}

    void serializeTo(void *buffer) const override{
        auto *req = (EncTypeDecRequestData *) buffer;
        req->common.reqType = reqType;

        COPY(&req->ciphertext, ciphertext);
        // req->ciphertext = *ciphertext;
    }

    void copyResultFrom(void *buffer)const override {
        auto *req = (EncTypeDecRequestData *) buffer;

        COPY(res, &req->plaintext);
        // *res = req->plaintext;
    }
};

template<typename Type1, typename Type2,int reqType>
class OneArgRequest: public Request {
public:
    DEFINE_ENCTYPE_1ARG_ReqData(Type1, Type2,OneArgRequestData);
    Type1 *in;
    Type2 *res;
    
    OneArgRequest(Type1 *in, Type2 *res) : in(in), res(res) {}

    void serializeTo(void *buffer) const override{
        auto *req = (OneArgRequestData *) buffer;
        req->common.reqType = reqType;
        req->in = *in;
    }

    void copyResultFrom(void *buffer)const override {
        auto *req = (OneArgRequestData *) buffer;
        *res = req->res;
    }
};


template<typename Type1, 
         typename Type2,
         typename Type3,
         typename resType,
         int reqType>
class ThreeArgRequest: public Request {
public:
    DEFINE_ENCTYPE_3ARG_ReqData(Type1,arg1,Type2,arg2, Type3,arg3, resType, ThreeArgRequestData);
    Type1 *arg1;
    Type2 *arg2;
    Type3 *arg3;
    resType *res;
    
    ThreeArgRequest(Type1 *arg1, Type2 *arg2,Type3 *arg3, resType *res) : arg1(arg1),arg2(arg2),arg3(arg3), res(res) {}

    void serializeTo(void *buffer) const override{
        auto *req = (ThreeArgRequestData *) buffer;
        req->common.reqType = reqType;
        COPY(&req->arg1, arg1);
        // req->arg1 = *arg1;
        req->arg2 = *arg2;
        req->arg3 = *arg3;
    }

    void copyResultFrom(void *buffer)const override {
        auto *req = (ThreeArgRequestData *) buffer;

        COPY(res, &req->res);
        // *res = req->res;
    }
};
