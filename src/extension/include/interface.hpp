#pragma once 

#include <request.hpp>
#include <request_types.h>

class TEEInvoker {
private:
    TEEInvoker();
    static TEEInvoker* invoker;
    void *req_buffer;
public: 
    ~TEEInvoker();
    static TEEInvoker *getInstance(){
        if (invoker == nullptr){
            invoker = new TEEInvoker;
        }
        return invoker;
    }
    /* send request, get result in req->res, get resp as return value.*/
    int sendRequest(Request *req);

};

#ifdef __x86_64 
#define YIELD_PROCESSOR __asm__ volatile("pause")
#elif __aarch64__
#define YIELD_PROCESSOR __asm__ volatile("yield")
#endif
