#include "enc_ops.hpp"


static char buf_left[1000];
static char buf_right[1000];
int enc_text_cmp(uint64_t left, uint64_t right){
    const char *l = getText(left, buf_left);
    const char *r = getText(right, buf_right);
    return strcmp(l, r);
}

int enc_text_like(uint64_t text, uint64_t pattern){
    const char *t = getText(text, buf_left);
    const char *p = getText(pattern, buf_right);
    std::cout << t << " " << p << std::endl;

    return MatchText(t, strlen(t), p, strlen(p));
}

uint64_t enc_text_concat(uint64_t left, uint64_t right){
    const char *l = getText(left, buf_left);
    const char *r = getText(right, buf_right);
    char *res = (char *)malloc(strlen(l) + strlen(r) + 1);
    strcpy(res, l);
    strcat(res, r);
    int key = insertStr(res);
    free(res);
    return makeIndex(0, key);
}

uint64_t enc_text_substr(uint64_t text, uint64_t start, uint64_t length){
    const char *t = getText(text, buf_left);
    int s = getInt(start) - 1;
    int l = getInt(length);
    char *res = (char *)malloc(l + 1);
    strncpy(res, t + s, l);
    res[l] = '\0';
    int key = insertStr(res);
    free(res);
    return makeIndex(0, key);
}
