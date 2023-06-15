#include "extension.h"

int MatchText(char* t, int tlen, char* p, int plen);

/*
 * cstring_to_text_with_len
 *
 * Same as cstring_to_text except the caller specifies the string length;
 * the string need not be null_terminated.
 */
text* cstring_to_text_with_len(const char* s, int len)
{
    text* result = (text*)palloc0(len + VARHDRSZ);

    SET_VARSIZE(result, len + VARHDRSZ);
    memcpy(VARDATA(result), s, len);

    return result;
}

// The input function converts a string to an enc_text element.
// @input: string
// @return: pointer to a structure describing enc_text element.
PG_FUNCTION_INFO_V1(pg_enc_text_in);
Datum
    pg_enc_text_in(PG_FUNCTION_ARGS)
{
    char* s = PG_GETARG_CSTRING(0);
    EncText *result;
    result = (EncText *) cstring_to_text_with_len(s, strlen(s)+1);
    PG_RETURN_POINTER(result);
}

// The output function converts an enc_text element to a string.
// @input: pointer to a structure describing enc_text element
// @return: string
PG_FUNCTION_INFO_V1(pg_enc_text_out);
Datum
    pg_enc_text_out(PG_FUNCTION_ARGS)
{
    // ereport(INFO, (errmsg("pg_enc_text_out here!")));
    EncText* s = PG_GETARG_ENCTEXT_P(0);

    PG_RETURN_CSTRING(TextDatumGetCString(s));
}

// @input: two strings
// @return: true, if strings are equal
//       false, otherwise
PG_FUNCTION_INFO_V1(pg_enc_text_eq);
Datum
    pg_enc_text_eq(PG_FUNCTION_ARGS)
{
    EncText* s1 = PG_GETARG_TEXT_PP(0);   
    EncText* s2 = PG_GETARG_TEXT_PP(1);
    char* str1 = VARDATA(s1);
    char* str2 = VARDATA(s2);

    PG_RETURN_BOOL(0 == strcmp(str1, str2));
}

// @input: two strings
// @return: true, if strings are not equal
//       false, otherwise
PG_FUNCTION_INFO_V1(pg_enc_text_ne);
Datum
    pg_enc_text_ne(PG_FUNCTION_ARGS)
{
    EncText* s1 = PG_GETARG_TEXT_PP(0);   
    EncText* s2 = PG_GETARG_TEXT_PP(1);
    char* str1 = VARDATA(s1);
    char* str2 = VARDATA(s2);

    PG_RETURN_BOOL(0 != strcmp(str1, str2));
}

// @input: two strings
// @return: true, if the first string is less or equal than the second one.
//       false, otherwise
PG_FUNCTION_INFO_V1(pg_enc_text_le);
Datum
    pg_enc_text_le(PG_FUNCTION_ARGS)
{
    EncText* s1 = PG_GETARG_TEXT_PP(0);   
    EncText* s2 = PG_GETARG_TEXT_PP(1);
    char* str1 = VARDATA(s1);
    char* str2 = VARDATA(s2);
    bool cmp = false;
    int ans = 0;
    ans = strcmp(str1, str2);

    if (ans <= 0)
        cmp = true;

    PG_RETURN_BOOL(cmp);
}

// @input: two strings
// @return: true, if the first string is less than the second one.
//       false, otherwise
PG_FUNCTION_INFO_V1(pg_enc_text_lt);
Datum
    pg_enc_text_lt(PG_FUNCTION_ARGS)
{
    EncText* s1 = PG_GETARG_TEXT_PP(0);   
    EncText* s2 = PG_GETARG_TEXT_PP(1);
    char* str1 = VARDATA(s1);
    char* str2 = VARDATA(s2);
    bool cmp = false;
    int ans = 0;
    ans = strcmp(str1, str2);

    if (ans < 0)
        cmp = true;

    PG_RETURN_BOOL(cmp);
}

// @input: two strings
// @return: true, if the first string is greater or equal than the second one.
//       false, otherwise
PG_FUNCTION_INFO_V1(pg_enc_text_ge);
Datum
    pg_enc_text_ge(PG_FUNCTION_ARGS)
{
    EncText* s1 = PG_GETARG_TEXT_PP(0);   
    EncText* s2 = PG_GETARG_TEXT_PP(1);
    char* str1 = VARDATA(s1);
    char* str2 = VARDATA(s2);
    bool cmp = false;
    int ans = 0;
    ans = strcmp(str1, str2);

    if (ans >= 0)
        cmp = true;

    PG_RETURN_BOOL(cmp);
}

// @input: two strings
// @return: true, if the first string is greater than the second one.
//       false, otherwise
PG_FUNCTION_INFO_V1(pg_enc_text_gt);
Datum
    pg_enc_text_gt(PG_FUNCTION_ARGS)
{
    EncText* s1 = PG_GETARG_TEXT_PP(0);   
    EncText* s2 = PG_GETARG_TEXT_PP(1);
    char* str1 = VARDATA(s1);
    char* str2 = VARDATA(s2);
    bool cmp = false;
    int ans = 0;
    ans = strcmp(str1, str2);

    if (ans > 0)
        cmp = true;

    PG_RETURN_BOOL(cmp);
}

// @input: two strings
// @return: -1, if s1 < s2,
//        0, if s1 = s2,
//        1, if s1 > s2
PG_FUNCTION_INFO_V1(pg_enc_text_cmp);
Datum
    pg_enc_text_cmp(PG_FUNCTION_ARGS)
{
    EncText* s1 = PG_GETARG_TEXT_PP(0);   
    EncText* s2 = PG_GETARG_TEXT_PP(1);
    char* str1 = VARDATA(s1);
    char* str2 = VARDATA(s2);
    int ans = 0;
    ans = strcmp(str1, str2);
    PG_RETURN_INT32(ans);
}

// The function encrypts the input string.
// IT'S A DEBUG FUNCTION SHOULD BE DELETED IN THE PRODUCT
// !!!!!!!!!!!!!!!!!!!!!!!!!
PG_FUNCTION_INFO_V1(pg_enc_text_encrypt);
Datum
    pg_enc_text_encrypt(PG_FUNCTION_ARGS)
{
    char* s = PG_GETARG_CSTRING(0);
    EncText *result;
    result = (EncText *) cstring_to_text_with_len(s, strlen(s));

    PG_RETURN_POINTER(result);
}

// The function decrypts the input enc_text element.
// IT'S A DEBUG FUNCTION SHOULD BE DELETED IN THE PRODUCT
// !!!!!!!!!!!!!!!!!!!!!!!!!
PG_FUNCTION_INFO_V1(pg_enc_text_decrypt);
Datum
    pg_enc_text_decrypt(PG_FUNCTION_ARGS)
{
    EncText* s1 = PG_GETARG_TEXT_PP(0);   
    // ereport(INFO, (errmsg("pg_enc_text_decrypt here!")));
    char* src = VARDATA(s1);
    size_t src_len = strlen(src);
    char* pDst = (char*)palloc((src_len + 1) * sizeof(char));
    memcpy(pDst, src, src_len);
    pDst[src_len] = '\0';
    PG_RETURN_CSTRING(pDst);
}

// @input: two strings
// @return: a result of a concatenation.
PG_FUNCTION_INFO_V1(pg_enc_text_concatenate);
Datum
    pg_enc_text_concatenate(PG_FUNCTION_ARGS)
{
    EncText* s1 = PG_GETARG_TEXT_PP(0);   
    EncText* s2 = PG_GETARG_TEXT_PP(1);
    char* str1 = VARDATA(s1);
    char* str2 = VARDATA(s2);
    strcat(str1, str2);

    EncText *result;
    result = (EncText *) cstring_to_text_with_len(str1, strlen(str1)+1);
    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(pg_enc_text_like);
Datum
    pg_enc_text_like(PG_FUNCTION_ARGS)
{
    EncText* s1 = PG_GETARG_ENCTEXT_P(0);   
    EncText* s2 = PG_GETARG_ENCTEXT_P(1);

    char* str = VARDATA(s1);
    char* pattern = VARDATA(s2);
    bool result = false;
    result = MatchText(str, strlen(str), pattern, strlen(pattern));
    PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(pg_enc_text_notlike);
Datum
    pg_enc_text_notlike(PG_FUNCTION_ARGS)
{
    EncText* s1 = PG_GETARG_TEXT_PP(0);   
    EncText* s2 = PG_GETARG_TEXT_PP(1);
    char* str = VARDATA(s1);
    char* pattern = VARDATA(s2);
    bool result = 0;
    result = MatchText(str, strlen(str), pattern, strlen(pattern));
    PG_RETURN_BOOL(1 ^ result);
}

// @input: string and two integers
// @return: the substring specified by from and to.
static inline int bytearray2int(uint8_t* pSrc, int *dst, size_t srcLen)
{
    if (srcLen < INT32_LENGTH)
        return 1;

    memcpy(dst, pSrc, INT32_LENGTH);

    return 0;
}

PG_FUNCTION_INFO_V1(substring);
Datum
    substring(PG_FUNCTION_ARGS)
{
    char* str = PG_GETARG_CSTRING(0);
    char* from = PG_GETARG_CSTRING(1);
    char* leng = PG_GETARG_CSTRING(2);
    int f, l;
    bytearray2int(from, &f, INT32_LENGTH);
    bytearray2int(leng, &l, INT32_LENGTH);
    str = VARDATA(str);
    char* res = palloc((l+1) * sizeof(char));
    strncpy(res, str+f-1, l);
    EncText *result;
    result = (EncText *) cstring_to_text_with_len(res, l);
    PG_RETURN_POINTER(result);
}

// The input function converts a string to an enc_text element.
// @input: varying char
// @return: pointer to a structure describing enc_text element.
PG_FUNCTION_INFO_V1(varchar_to_enc_text);
Datum
    varchar_to_enc_text(PG_FUNCTION_ARGS)
{
    Datum txt = PG_GETARG_DATUM(0);
    char* s = TextDatumGetCString(txt);
    EncText *result;
    result = (EncText *) cstring_to_text_with_len(s, strlen(s));
    PG_RETURN_POINTER(result);
}
