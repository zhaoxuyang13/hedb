#include "enc_ops.hpp"

uint64_t enc_int_add(uint64_t left, uint64_t right){
    int l = getInt(left);
    int r = getInt(right);
    int key = insertInt(l + r);
    return makeIndex(0, key);
}

uint64_t enc_int_sub(uint64_t left, uint64_t right){
    int l = getInt(left);
    int r = getInt(right);
    int key = insertInt(l - r);
    return makeIndex(0, key);
}

uint64_t enc_int_mult(uint64_t left, uint64_t right){
    int l = getInt(left);
    int r = getInt(right);
    int key = insertInt(l * r);
    return makeIndex(0, key);
}

uint64_t enc_int_div(uint64_t left, uint64_t right){
    int l = getInt(left);
    int r = getInt(right);
    int key = insertInt(l / r);
    return makeIndex(0, key);
}

uint64_t enc_int_pow(uint64_t left, uint64_t right){
    int l = getInt(left);
    int r = getInt(right);
    int key = insertInt(pow(l, r));
    return makeIndex(0, key);
}

uint64_t enc_int_mod(uint64_t left, uint64_t right){
    int l = getInt(left);
    int r = getInt(right);
    int key = insertInt(l % r);
    return makeIndex(0, key);
}

int enc_int_cmp(uint64_t left, uint64_t right){
    int l = getInt(left);
    int r = getInt(right);
    return l > r ? 1 : (l < r ? -1 : 0);
}

uint64_t enc_int_sum_bulk(size_t bulk_size, uint64_t *bulk_data){
    int sum = 0;
    for(int i = 0; i < bulk_size; i++){
        sum += getInt(bulk_data[i]);
    }
    int key = insertInt(sum);
    return makeIndex(0, key);
}

