#include "kv.hpp"


KVStore *kvStore;
const uint64_t MB = 1024 * 1024;
const uint64_t GB = 1024 * MB;

void init_kv(const char *filename){
    if(kvStore == nullptr){
        static managed_mapped_file segment(open_only, filename); //  4 * GB
        kvStore = segment.find<KVStore>("kv").first;
    }
}