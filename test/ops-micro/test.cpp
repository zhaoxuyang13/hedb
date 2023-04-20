#include <chrono>
#include <ctime>
#include <enc_float_ops.hpp>
#include <enc_int_ops.hpp>
#include <enc_text_ops.hpp>
#include <enc_timestamp_ops.hpp>
#include <interface.hpp>
#include <iostream>
#include <request.hpp>
#include <sched.h>
#include <stdarg.h>

using namespace std;
// void print_info(char const* str, ...){
//     cout << str << endl;
// }

int get_enc_int(int val, EncInt* enc)
{
    TEEInvoker* invoker = TEEInvoker::getInstance();
    auto req = new EncRequest<int, EncInt, CMD_INT_ENC>(&val, enc);
    int resp = invoker->sendRequest(req);
    return resp;
}
int get_dec_int(EncInt* enc, int* val)
{
    TEEInvoker* invoker = TEEInvoker::getInstance();
    auto req = new DecRequest<EncInt, int, CMD_INT_DEC>(enc, val);
    int resp = invoker->sendRequest(req);
    return resp;
}

inline void issue_cmp_req(EncInt left, EncInt right, int& cmp)
{
    TEEInvoker* invoker = TEEInvoker::getInstance();
    auto req = CmpRequest<EncInt, CMD_INT_CMP>(&left, &right, &cmp);
    int resp = invoker->sendRequest(&req);
}
inline void issue_calc_req(EncInt left, EncInt right, EncInt& res)
{
    TEEInvoker* invoker = TEEInvoker::getInstance();
    auto req = CalcRequest<EncInt>(CMD_INT_PLUS, &left, &right, &res);
    int resp = invoker->sendRequest(&req);
}
int main()
{
    cpu_set_t mask;
    CPU_ZERO(&mask);
    for (int i = 0; i < 4; i++)
        CPU_SET(i, &mask);
    int result = sched_setaffinity(0, sizeof(mask), &mask);

    EncInt left, right, res;
    int resp = 0;
    int cmp;
    get_enc_int(1, &left);
    get_enc_int(2, &right);

    auto start = std::chrono::system_clock::now();
    for (int i = 0; i <= 10000000; i++) {
        issue_cmp_req(left, right, cmp);
        // issue_calc_req(left,right,res);
    }
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "cmp average time: " << elapsed_seconds.count() << "s"
              << std::endl;

    // int val = 0;
    // get_dec_int(&res, &val);

    // cout << res << endl;
    cout << cmp << endl;
}