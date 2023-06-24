#include "enc_ops.hpp"

uint64_t enc_float_add(uint64_t left, uint64_t right){
    float l = getFloat(left);
    float r = getFloat(right);
    int key = insertFloat(l + r);
    return makeIndex(0, key);
}

uint64_t enc_float_sub(uint64_t left, uint64_t right){
    float l = getFloat(left);
    float r = getFloat(right);
    int key = insertFloat(l - r);
    return makeIndex(0, key);
}

uint64_t enc_float_mult(uint64_t left, uint64_t right){
    float l = getFloat(left);
    float r = getFloat(right);
    int key = insertFloat(l * r);
    return makeIndex(0, key);
}

uint64_t enc_float_div(uint64_t left, uint64_t right){
    float l = getFloat(left);
    float r = getFloat(right);
    int key = insertFloat(l / r);
    return makeIndex(0, key);
}

uint64_t enc_float_pow(uint64_t left, uint64_t right){
    float l = getFloat(left);
    float r = getFloat(right);
    int key = insertFloat(pow(l, r));
    return makeIndex(0, key);
}

uint64_t enc_float_mod(uint64_t left, uint64_t right){
    float l = getFloat(left);
    float r = getFloat(right);
    int key = insertFloat(fmod(l, r));
    return makeIndex(0, key);
}

int enc_float_cmp(uint64_t left, uint64_t right){
    float l = getFloat(left);
    float r = getFloat(right);
    return l > r ? 1 : (l < r ? -1 : 0);
}

uint64_t enc_float_sum_bulk(size_t bulk_size, uint64_t *bulk_data){
    double sum = 0;
    for(int i = 0; i < bulk_size; i++){
        sum += getFloat(bulk_data[i]);
    }
    int key = insertFloat(sum);
    return makeIndex(0, key);
}
