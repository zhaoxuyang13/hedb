#pragma once
#include <stdint.h>
#include <kv-store.hpp>

const char FLAG_CHAR = '$';


inline uint32_t getMapid(uint64_t a){
    return a >> 32;
}
inline uint32_t getKey(uint64_t a){
    return a & 0xffffffff;
}
inline uint64_t makeIndex(uint32_t mapid, uint32_t key){
    return ((uint64_t)mapid << 32) | key;
}


extern KVStore kv;

inline int32_t getInt(uint64_t a){
    return kv.find_int(getMapid(a), getKey(a));
}
inline float getFloat(uint64_t a){
    return kv.find_float(getMapid(a), getKey(a));
}
inline TIMESTAMP getTs(uint64_t a){
    return kv.find_ts(getMapid(a), getKey(a));
}
inline std::string& getStr(uint64_t a){
    uint32_t mapid = getMapid(a);
    if(mapid == 0){
        static std::string tmp = "";
        tmp = kv.find_str_tmp(getKey(a));
        return tmp;
    }else {
        return kv.find_str(mapid, getKey(a));
    }
}
inline const char *getText(uint64_t a){
    return getStr(a).c_str();
}

inline uint32_t insertInt(int value){
    return kv.push_back(0, value);
}
inline uint32_t insertFloat(float value){
    return kv.push_back(0, value);
}
inline uint32_t insertTs(TIMESTAMP value){
    return kv.push_back(0, value);
}
inline uint32_t insertStr(const char *value){
    return kv.push_back(0, value);
}