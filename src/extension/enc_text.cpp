#include "extension.hpp"

extern "C" {
PG_FUNCTION_INFO_V1(pg_enc_text_cmp);
PG_FUNCTION_INFO_V1(pg_enc_text_gt);
PG_FUNCTION_INFO_V1(pg_enc_text_ge);
PG_FUNCTION_INFO_V1(pg_enc_text_lt);
PG_FUNCTION_INFO_V1(pg_enc_text_ne);
PG_FUNCTION_INFO_V1(pg_enc_text_le);
PG_FUNCTION_INFO_V1(pg_enc_text_eq);
PG_FUNCTION_INFO_V1(pg_enc_text_out);
PG_FUNCTION_INFO_V1(pg_enc_text_in);
PG_FUNCTION_INFO_V1(substring);
PG_FUNCTION_INFO_V1(pg_enc_text_like);
PG_FUNCTION_INFO_V1(pg_enc_text_notlike);
PG_FUNCTION_INFO_V1(pg_enc_text_concatenate);

PG_FUNCTION_INFO_V1(varchar_to_enc_text);

PG_FUNCTION_INFO_V1(pg_enc_text_encrypt);
PG_FUNCTION_INFO_V1(pg_enc_text_decrypt);
}

int MatchText(char* t, int tlen, char* p, int plen);

text* cstring_to_text_with_len(const char* s, int len)
{
    text* result = (text*)palloc0(len + VARHDRSZ + 1);
    SET_VARSIZE(result, len + VARHDRSZ + 1);
    strcpy(VARDATA(result), s);
    return result;
}

Datum
    pg_enc_text_in(PG_FUNCTION_ARGS)
{
    char* s = PG_GETARG_CSTRING(0);
    uint64_t index = 0;
    if(s[0] == FLAG_CHAR){
        index = strtoull(s + 1, NULL, 10);
    }else {
        index = makeIndex(0, insertStr(s));
        char tmp[1000];
        const char *tmps = getText(index, tmp);
        ereport(INFO, 
            (errmsg("insert temp key: %s, %ld, %s", s, index, tmps)));

    }
    PG_RETURN_DATUM(index);
}

Datum
    pg_enc_text_out(PG_FUNCTION_ARGS)
{
    uint64_t index = PG_GETARG_DATUM(0);
    char *res = (char *)malloc(1000);
    const char *s = getText(index, res);
    if(getMapid(index) != 0){
        strcpy(res, s);
    }
    PG_RETURN_CSTRING(res);
}

Datum
    pg_enc_text_eq(PG_FUNCTION_ARGS)
{
    uint64_t left = PG_GETARG_DATUM(0);
    uint64_t right = PG_GETARG_DATUM(1);
    int cmp = enc_text_cmp(left, right);

    PG_RETURN_BOOL(0 == cmp);
}

Datum
    pg_enc_text_ne(PG_FUNCTION_ARGS)
{
    uint64_t left = PG_GETARG_DATUM(0);
    uint64_t right = PG_GETARG_DATUM(1);
    int cmp = enc_text_cmp(left, right);

    PG_RETURN_BOOL(0 != cmp);
}

Datum
    pg_enc_text_le(PG_FUNCTION_ARGS)
{
    uint64_t left = PG_GETARG_DATUM(0);
    uint64_t right = PG_GETARG_DATUM(1);
    int cmp = enc_text_cmp(left, right);

    PG_RETURN_BOOL(0 >= cmp);
}

Datum
    pg_enc_text_lt(PG_FUNCTION_ARGS)
{
    uint64_t left = PG_GETARG_DATUM(0);
    uint64_t right = PG_GETARG_DATUM(1);
    int cmp = enc_text_cmp(left, right);

    PG_RETURN_BOOL(0 > cmp);
}

Datum
    pg_enc_text_ge(PG_FUNCTION_ARGS)
{
    uint64_t left = PG_GETARG_DATUM(0);
    uint64_t right = PG_GETARG_DATUM(1);
    int cmp = enc_text_cmp(left, right);

    PG_RETURN_BOOL(0 <= cmp);
}


Datum
    pg_enc_text_gt(PG_FUNCTION_ARGS)
{
    uint64_t left = PG_GETARG_DATUM(0);
    uint64_t right = PG_GETARG_DATUM(1);
    int cmp = enc_text_cmp(left, right);

    PG_RETURN_BOOL(0 < cmp);
}

Datum
    pg_enc_text_cmp(PG_FUNCTION_ARGS)
{
    uint64_t left = PG_GETARG_DATUM(0);
    uint64_t right = PG_GETARG_DATUM(1);
    int cmp = enc_text_cmp(left, right);

    PG_RETURN_INT32(cmp);
}



Datum
    pg_enc_text_encrypt(PG_FUNCTION_ARGS)
{
    PG_RETURN_DATUM(0);
}

Datum
    pg_enc_text_decrypt(PG_FUNCTION_ARGS)
{
    PG_RETURN_DATUM(0);
}

Datum
    pg_enc_text_concatenate(PG_FUNCTION_ARGS)
{
    uint64_t left = PG_GETARG_DATUM(0);
    uint64_t right = PG_GETARG_DATUM(1);
    uint64_t res = enc_text_concat(left, right);

    PG_RETURN_DATUM(res);
}

Datum
    pg_enc_text_like(PG_FUNCTION_ARGS)
{
    uint64_t left = PG_GETARG_DATUM(0);
    uint64_t right = PG_GETARG_DATUM(1);
    int res = enc_text_like(left, right);

    PG_RETURN_BOOL(res);
}
Datum
    pg_enc_text_notlike(PG_FUNCTION_ARGS)
{
    uint64_t left = PG_GETARG_DATUM(0);
    uint64_t right = PG_GETARG_DATUM(1);
    int res = enc_text_like(left, right);

    PG_RETURN_BOOL(res == false);
}


Datum
    substring(PG_FUNCTION_ARGS)
{
    uint64_t str = PG_GETARG_DATUM(0);
    int32_t from = PG_GETARG_INT32(1);
    int32_t len = PG_GETARG_INT32(2);
    uint64_t res = enc_text_substr(str, from, len);

    PG_RETURN_DATUM(res);
}


Datum
    varchar_to_enc_text(PG_FUNCTION_ARGS)
{
    Datum txt = PG_GETARG_DATUM(0);
    char* s = TextDatumGetCString(txt);
    uint64_t index = makeIndex(0, insertStr(s));
    PG_RETURN_DATUM(index);
}
