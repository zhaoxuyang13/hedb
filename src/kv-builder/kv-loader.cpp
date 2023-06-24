#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <scoped_allocator>
#include <cstdlib> //std::system
#include <iostream>
#include <kv.hpp>


using namespace boost::interprocess;

// const uint64_t MB = 1024 * 1024;
// const uint64_t GB = 1024 * MB;
const char * MMAP_FILENAME = "/home/xuyang/Codes/tmp/HEDB/hedb-extension/kv.mmap";

int main(int argc, char *argv[])
{
    init_kv(MMAP_FILENAME);
    if(kvStore == nullptr){
        std::cout << "KVStore not found, " << std::endl;
        return 0;
    }
    
    
    for(auto& kv : kvStore->getIntKV()){
        std::cout << "IntKV: " ;
        std::cout << kv.size();
        std::cout << std::endl;
    }
    for(auto& kv : kvStore->getFloatKV()){
        std::cout << "FloatKV: " ;
        std::cout << kv.size();
        std::cout << std::endl;
    }
    for(auto& kv : kvStore->getTSKV()){
        std::cout << "TimestampKV: " ;
        std::cout << kv.size();
        std::cout << std::endl;
    }
    for(auto& kv : kvStore->getStrKV()){
        std::cout << "StringKV: " ;
        std::cout << kv.size();
        for(int i = 0; i < 5 && i< kv.size(); i ++){
            std::cout << kv[i].c_str() << " ";
        }
        std::cout << std::endl;
    }

}
