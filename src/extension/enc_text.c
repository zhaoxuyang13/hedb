/*
 * a simple UDF for text
 */
#include "extension.h"


static bool MatchText(char *t, int tlen, char *p, int plen)
{

	char* subt = t;
	char* subp = p;
	int i = 0;
	int j = 0;
	while (i <= tlen - 1 && j <= plen - 1)
	{
		if (subt[i] == subp[j])
		{
			i++;
			j++;
		}
		else
		{
			i = i - j + 1;
			j = 0;
		}
	}
	if (j == strlen(subp))
	{
		return true;
	}
	return false;
}

static char* SubText(char *str, int from, int to)
{
    char* result = palloc((to-from+1)*sizeof(char));
    for (int i = 0; i < to-from+1; i++)
    {
        result[i] = str[from + i - 1];
    }
    return result;
}

// The input function converts a string to an enc_text element.
// @input: string
// @return: pointer to a structure describing enc_text element.
PG_FUNCTION_INFO_V1(pg_enc_text_in);
Datum
    pg_enc_text_in(PG_FUNCTION_ARGS)
{
    // ereport(INFO, (errmsg("pg_enc_text_in here!")));
    char* pSrc = PG_GETARG_CSTRING(0);
    int srcLen = strlen(pSrc);

    if (srcLen > STRING_LENGTH)
    {
        ereport(ERROR, (errmsg("Error: the length of the element is more than maximum")));
        PG_RETURN_CSTRING("");
    }
    PG_RETURN_CSTRING(pSrc);
}

// The output function converts an enc_text element to a string.
// @input: pointer to a structure describing enc_text element
// @return: string
PG_FUNCTION_INFO_V1(pg_enc_text_out);
Datum
    pg_enc_text_out(PG_FUNCTION_ARGS)
{
    // ereport(INFO, (errmsg("pg_enc_text_out here!")));
    char* pSrc = PG_GETARG_CSTRING(0);
    PG_RETURN_CSTRING(pSrc);
}

// @input: two strings
// @return: true, if strings are equal
//       false, otherwise
PG_FUNCTION_INFO_V1(pg_enc_text_eq);
Datum
    pg_enc_text_eq(PG_FUNCTION_ARGS)
{
    char* str1 = PG_GETARG_CSTRING(0);
    char* str2 = PG_GETARG_CSTRING(1);
    bool cmp = false;
    int ans = 0;
    ans = strcmp(str1, str2);

    if (ans == 0)
        cmp = true;

    PG_RETURN_BOOL(cmp);
}

// @input: two strings
// @return: true, if strings are not equal
//       false, otherwise
PG_FUNCTION_INFO_V1(pg_enc_text_ne);
Datum
    pg_enc_text_ne(PG_FUNCTION_ARGS)
{
    char* str1 = PG_GETARG_CSTRING(0);
    char* str2 = PG_GETARG_CSTRING(1);
    bool cmp = false;
    int ans = 0;
    ans = strcmp(str1, str2);

    if (ans != 0)
        cmp = true;

    PG_RETURN_BOOL(cmp);
}

// @input: two strings
// @return: true, if the first string is less or equal than the second one.
//       false, otherwise
PG_FUNCTION_INFO_V1(pg_enc_text_le);
Datum
    pg_enc_text_le(PG_FUNCTION_ARGS)
{
    char* str1 = PG_GETARG_CSTRING(0);
    char* str2 = PG_GETARG_CSTRING(1);
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
    char* str1 = PG_GETARG_CSTRING(0);
    char* str2 = PG_GETARG_CSTRING(1);
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
    char* str1 = PG_GETARG_CSTRING(0);
    char* str2 = PG_GETARG_CSTRING(1);
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
    char* str1 = PG_GETARG_CSTRING(0);
    char* str2 = PG_GETARG_CSTRING(1);
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
    char* str1 = PG_GETARG_CSTRING(0);
    char* str2 = PG_GETARG_CSTRING(1);
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
    // ereport(INFO, (errmsg("pg_enc_text_encrypt here!")));
    char* src = PG_GETARG_CSTRING(0);
    size_t src_len = strlen(src);
    char* pDst = (char*)palloc((src_len + 1) * sizeof(char));
    memcpy(pDst, src, src_len);
    pDst[src_len] = '\0';
    PG_RETURN_CSTRING(pDst);
}

// The function decrypts the input enc_text element.
// IT'S A DEBUG FUNCTION SHOULD BE DELETED IN THE PRODUCT
// !!!!!!!!!!!!!!!!!!!!!!!!!
PG_FUNCTION_INFO_V1(pg_enc_text_decrypt);
Datum
    pg_enc_text_decrypt(PG_FUNCTION_ARGS)
{
    // ereport(INFO, (errmsg("pg_enc_text_decrypt here!")));
    char* src = PG_GETARG_CSTRING(0);
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

    char* str1 = PG_GETARG_CSTRING(0);
    char* str2 = PG_GETARG_CSTRING(1);
    strcat(str1, str2);
    PG_RETURN_CSTRING(str1);
}

PG_FUNCTION_INFO_V1(pg_enc_text_like);
Datum
    pg_enc_text_like(PG_FUNCTION_ARGS)
{
    char* str = PG_GETARG_CSTRING(0);
    char* pattern = PG_GETARG_CSTRING(1);
    bool result = false;
    result = MatchText(str, strlen(str), pattern, strlen(pattern));
    PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(pg_enc_text_notlike);
Datum
    pg_enc_text_notlike(PG_FUNCTION_ARGS)
{
    char* str = PG_GETARG_CSTRING(0);
    char* pattern = PG_GETARG_CSTRING(1);
    bool result = 0;
    result = MatchText(str, strlen(str), pattern, strlen(pattern));
    PG_RETURN_BOOL(1 ^ result);
}

// @input: string and two integers
// @return: the substring specified by from and to.
PG_FUNCTION_INFO_V1(substring);
Datum
    substring(PG_FUNCTION_ARGS)
{
    // NOT BE INVOKED
    // char* str = PG_GETARG_CSTRING(0);
    // char* from = PG_GETARG_CSTRING(1);
    // char* to = PG_GETARG_CSTRING(2);
    // int f, t;
    // bytearray2int(from, &f, INT32_LENGTH);
    // bytearray2int(to, &t, INT32_LENGTH);
    // char* result = palloc((t-f+1) * sizeof(char));
    // result = SubText(str, f, t);
    // PG_RETURN_CSTRING(result);
}

// The input function converts a string to an enc_text element.
// @input: varying char
// @return: pointer to a structure describing enc_text element.
PG_FUNCTION_INFO_V1(varchar_to_enc_text);
Datum
    varchar_to_enc_text(PG_FUNCTION_ARGS)
{
    Datum txt = PG_GETARG_DATUM(0);
    char* src = TextDatumGetCString(txt);
    int len = strlen(src);
    char* pDst = (char*)palloc((STRING_LENGTH) * sizeof(char));

    if (len > STRING_LENGTH - 1)
    {
        ereport(ERROR, (errmsg("Error: the length of the element is more than maximum")));
        PG_RETURN_CSTRING("");
    }

    memcpy(pDst, src, len);
    pDst[len] = '\0';
    PG_RETURN_CSTRING(pDst);
}
