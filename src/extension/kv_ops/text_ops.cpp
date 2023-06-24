#include "enc_ops.hpp"

int enc_text_cmp(uint64_t left, uint64_t right){
    const char *l = getText(left);
    const char *r = getText(right);
    std::cout << l << " " << r << std::endl;
    
    return strcmp(l, r);
}

int enc_text_like(uint64_t text, uint64_t pattern){
    const char *t = getText(text);
    const char *p = getText(pattern);
    return MatchText(t, strlen(t), p, strlen(p));
}

uint64_t enc_text_concat(uint64_t left, uint64_t right){
    const char *l = getText(left);
    const char *r = getText(right);
    char *res = (char *)malloc(strlen(l) + strlen(r) + 1);
    strcpy(res, l);
    strcat(res, r);
    int key = insertStr(res);
    free(res);
    return makeIndex(0, key);
}

uint64_t enc_text_substr(uint64_t text, uint64_t start, uint64_t length){
    const char *t = getText(text);
    char *res = (char *)malloc(length + 1);
    strncpy(res, t + start, length);
    res[length] = '\0';
    int key = insertStr(res);
    free(res);
    return makeIndex(0, key);
}


