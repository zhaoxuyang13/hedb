#pragma once 
#include <stdafx.h>

inline int plain_int_add(int left, int right){
    return left + right;
}
inline int plain_int_minus(int left, int right){
    return left - right;
}
inline int plain_int_mult(int left, int right){
    return left * right;
}
inline int plain_int_div(int left, int right){
    return left / right;
}
inline int plain_int_mod(int left, int right){
    return left % right;
}
inline int plain_int_pow(int left, int right){
    return pow(left, right);
}
inline int plain_int_cmp(int left, int right){
    return left - right;
}
inline int plain_int_bulk_sum(int size, int *array){
    int sum = 0;
    for(int i = 0; i < size; i++){
        sum += array[i];
    }
    return sum;
}
inline int plain_int_bulk_avg(int size, int *array){
    int sum = 0;
    for(int i = 0; i < size; i++){
        sum += array[i];
    }
    return sum / size;
}

/* float */
inline float plain_float_add(float left, float right){
    return left + right;
}
inline float plain_float_minus(float left, float right){
    return left - right;
}
inline float plain_float_mult(float left, float right){
    return left * right;
}
inline float plain_float_div(float left, float right){
    return left / right;
}
inline float plain_float_exp(float left, float right){
    return pow(left, right);
}
inline int plain_float_cmp(float left, float right){
    return left > right ? 1 : (left < right ? -1 : 0);
}
double plain_float_bulk_sum(int size, float *array){
    double sum = 0;
    for(int i = 0; i < size; i++){
        sum += array[i];
    }
    return sum;
}
double plain_float_bulk_avg(int size, float *array){
    double sum = 0;
    for(int i = 0; i < size; i++){
        sum += array[i];
    }
    return sum / size;
}

/* text */
inline int plain_text_cmp(char *lhs, uint32_t l_size, char *rhs, uint32_t r_size){
    return strcmp(lhs, rhs);
}
int MatchText(char* t, int tlen, char* p, int plen);
inline int plain_text_like(char *lhs, uint32_t l_size, char *rhs, uint32_t r_size){
    return MatchText(lhs, l_size, rhs, r_size);
}
inline void plain_text_concat(char *left, uint32_t l_size, char *right, uint32_t r_size){
    strcat(left, right);
}
inline int plain_text_substring(char *str, int begin, int length, char *substr){
    strncpy(substr, str + begin, length);
    return 0;
}

/* plain timestamp */
inline int plain_timestamp_cmp(TIMESTAMP left, TIMESTAMP right){
    return left > right ? 1 : (left < right ? -1 : 0);
}

inline int plain_timestamp_extract_year(int64_t timestamp)
{

    int64_t date;
    unsigned int quad;
    unsigned int extra;
    int year;

    TMODULO(timestamp, date, USECS_PER_DAY);
    if (timestamp < INT64CONST(0)) {
        timestamp += USECS_PER_DAY;
        date -= 1;
    }

    /* add offset to go from J2000 back to standard Julian date */
    date += POSTGRES_EPOCH_JDATE;

    /* Julian day routine does not work for negative Julian days */
    if (date < 0 || date > (int64_t)INT_MAX)
        return -1;

    date += 32044;
    quad = date / 146097;
    extra = (date - quad * 146097) * 4 + 3;

    date += 60 + quad * 3 + extra / 146097;
    quad = date / 1461;
    date -= quad * 1461;

    year = date * 4 / 1461;
    year += quad * 4;
    return year - 4800;
}
