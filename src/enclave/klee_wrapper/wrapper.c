#include <stdio.h>
#include <defs.h>
#include <klee/klee.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "prefix_udf.h"
#include "plain_text_ops.h"
#include "plain_int_ops.h"
#include "plain_timestamp_ops.h"


#define MAX_PREFIX_SIZE 100
int prefix_overlap_wrapper(){
    prefix_range *a = (prefix_range *)malloc(MAX_PREFIX_SIZE),
                 *b = (prefix_range *)malloc(MAX_PREFIX_SIZE);
    klee_make_symbolic(a, MAX_PREFIX_SIZE, "a");
    klee_make_symbolic(b, MAX_PREFIX_SIZE, "b");
    bool is_overlap = pr_overlaps(a,b);
    return 0; 
}

#define MAX_TEXT_LENGTH 100
int text_cmp_wrapper(){
    char *a = (char *)malloc(MAX_TEXT_LENGTH);
    char *b = (char *)malloc(MAX_TEXT_LENGTH);
    klee_make_symbolic(a, MAX_TEXT_LENGTH, "a");
    klee_make_symbolic(b, MAX_TEXT_LENGTH, "b");
    int cmp = plain_text_cmp(a, (uint32_t) strlen(a), b, strlen(b));
    return 0;
}

int text_like_wrapper(){
    char *a = (char *)malloc(MAX_TEXT_LENGTH);
    char *b = (char *)malloc(MAX_TEXT_LENGTH);
    klee_make_symbolic(a, MAX_TEXT_LENGTH, "a");
    klee_make_symbolic(b, MAX_TEXT_LENGTH, "b");
    int cmp = plain_text_like(a, strlen(a), b, strlen(b));
    return 0;
}

int text_concat_wrapper(){
    char *a = (char *)malloc(MAX_TEXT_LENGTH);
    char *b = (char *)malloc(MAX_TEXT_LENGTH);
    klee_make_symbolic(a, MAX_TEXT_LENGTH, "a");
    klee_make_symbolic(b, MAX_TEXT_LENGTH, "b");
    int len = strlen(a);
    plain_text_concat(a, (uint32_t*) &len, b, strlen(b));
    return 0;
}

int text_substring_wrapper(){
    char *a = (char *)malloc(MAX_TEXT_LENGTH);
    char *res = (char *)malloc(MAX_TEXT_LENGTH);
    int begin, end;
    klee_make_symbolic(a, MAX_TEXT_LENGTH, "str");
    klee_make_symbolic(&begin, 4, "begin");
    klee_make_symbolic(&end, 4, "end");
    int resp = plain_text_substring(a, strlen(a), begin, res);
    return 0;
}

int int_calc_wrapper(){
    int reqType, left, right;
    klee_make_symbolic(&reqType, 4, "type");
    klee_make_symbolic(&left, 4, "left");
    klee_make_symbolic(&right, 4, "right");
    plain_int32_calc(reqType, left, right);
    return 0;
}

int int_cmp_wrapper(){
    int left, right;
    klee_make_symbolic(&left, 4, "left");
    klee_make_symbolic(&right, 4, "right");
    plain_int32_cmp(left, right);
    return 0;
}

int int_bulk_wrapper(){
    int reqType, size; 
    int *array = (int *)malloc(sizeof(int) * size);
    klee_make_symbolic(&reqType, 4, "type");
    klee_make_symbolic(&size, 4, "size");
    klee_make_symbolic(&array, sizeof(int) * size, "array");
    plain_int32_bulk(reqType, size, array);
    return 0;
}