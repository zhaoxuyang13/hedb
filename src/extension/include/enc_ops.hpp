#pragma once 

#include "kv.hpp"

uint64_t enc_int_add(uint64_t left, uint64_t right);
uint64_t enc_int_sub(uint64_t left, uint64_t right);
uint64_t enc_int_mult(uint64_t left, uint64_t right);
uint64_t enc_int_div(uint64_t left, uint64_t right);
uint64_t enc_int_pow(uint64_t left, uint64_t right);
uint64_t enc_int_mod(uint64_t left, uint64_t right);
int enc_int_cmp(uint64_t left, uint64_t right);
uint64_t enc_int_sum_bulk(size_t bulk_size, uint64_t *bulk_data);


int enc_float_cmp(uint64_t left, uint64_t right);
uint64_t enc_float_add(uint64_t left, uint64_t right);
uint64_t enc_float_sub(uint64_t left, uint64_t right);
uint64_t enc_float_mult(uint64_t left, uint64_t right);
uint64_t enc_float_div(uint64_t left, uint64_t right);
uint64_t enc_float_pow(uint64_t left, uint64_t right);
uint64_t enc_float_mod(uint64_t left, uint64_t right);
uint64_t enc_float_sum_bulk(size_t bulk_size, uint64_t *bulk_data);

int enc_text_cmp(uint64_t left, uint64_t right);
int enc_text_like(uint64_t str, uint64_t pattern);
uint64_t enc_text_concat(uint64_t left, uint64_t right);
uint64_t enc_text_substr(uint64_t str, uint64_t begin, uint64_t length);
int MatchText(char* t, int tlen, char* p, int plen);

int enc_timestamp_cmp(uint64_t left, uint64_t right);
uint64_t enc_timestamp_extract_year(uint64_t in);
