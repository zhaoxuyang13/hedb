#include <enc_float_ops.hpp>
#include <enc_int_ops.hpp>
#include <enc_timestamp_ops.hpp>
#include <enc_text_ops.hpp>
#include <interface.hpp>
#include <request.hpp>
#include <iostream>

using namespace std;
void print_info(char const* str, ...){
    cout << str << endl;
}

int get_enc_int(int val, EncInt *enc){
    TEEInvoker *invoker = TEEInvoker::getInstance();
    auto req = new EncRequest<int, EncInt, CMD_INT_ENC>(&val, enc);
    int resp = invoker->sendRequest(req);
    return resp;
}
int get_dec_int(EncInt *enc, int *val){
    TEEInvoker *invoker = TEEInvoker::getInstance();
    auto req = new DecRequest<EncInt,int, CMD_INT_DEC>(enc, val);
    int resp = invoker->sendRequest(req);
    return resp;
}
int main(){
    EncInt left, right, res;
    int resp = 0;
    
    get_enc_int(1, &left);
    get_enc_int(2, &right);

    TEEInvoker *invoker = TEEInvoker::getInstance();
    auto *req = new CalcRequest<EncInt>(CMD_INT_PLUS, &left, &right, &res);
    
    resp = invoker->sendRequest(req);
    int val = 0;
    get_dec_int(&res, &val);
    
    cout << val << endl;
}