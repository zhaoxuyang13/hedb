#include <extension.hpp>
#include <kv.hpp>
#include <iostream>

using std::cout, std::endl;
using namespace boost::interprocess;
const char * MMAP_FILENAME = "/home/xuyang/Codes/tmp/HEDB/hedb-extension/kv.mmap";


extern "C" {
PG_MODULE_MAGIC;
PG_FUNCTION_INFO_V1(launch);
// defined _PG_init
void _PG_init(void);
}


void _PG_init(void){
    if(kvStore == nullptr){
        init_kv(MMAP_FILENAME);
    }  

    if(kvStore == nullptr){
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
            errmsg("KVStore not found, ")));
    }else {
        ereport(INFO, (errcode(ERRCODE_SUCCESSFUL_COMPLETION),
                errmsg("KVStore loaded %p", kvStore)));
    }

}

Datum
    launch(PG_FUNCTION_ARGS){
    if(kvStore == nullptr){
        init_kv(MMAP_FILENAME);
    }

    if(kvStore == nullptr){
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                errmsg("KVStore not found, ")));
    }else {
        ereport(INFO, (errcode(ERRCODE_SUCCESSFUL_COMPLETION),
                errmsg("KVStore loaded %p", kvStore)));
    }

    for(auto& k : kvStore->getIntKV()){
        std::cout << "IntKV: " ;
        std::cout << k.size();
        std::cout << std::endl;
    }
    for(auto& k : kvStore->getFloatKV()){
        std::cout << "FloatKV: " ;
        std::cout << k.size();
        std::cout << std::endl;
    }
    for(auto& k : kvStore->getTSKV()){
        std::cout << "TimestampKV: " ;
        std::cout << k.size();
        std::cout << std::endl;
    }
    for(auto& k : kvStore->getStrKV()){
        std::cout << "StringKV: " ;
        std::cout << k.size();
    }


    PG_RETURN_VOID();
}