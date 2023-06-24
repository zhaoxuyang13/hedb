/*
 * a simple UDF for float4
 */
#include "extension.hpp"
#include <string.h>


extern "C" {
PG_FUNCTION_INFO_V1(pg_enc_float4_in);
PG_FUNCTION_INFO_V1(pg_enc_float4_out);
PG_FUNCTION_INFO_V1(pg_enc_float4_sum_bulk);
PG_FUNCTION_INFO_V1(pg_enc_float4_avg_bulk);
PG_FUNCTION_INFO_V1(pg_enc_float4_add);
PG_FUNCTION_INFO_V1(pg_enc_float4_sub);
PG_FUNCTION_INFO_V1(pg_enc_float4_mult);
PG_FUNCTION_INFO_V1(pg_enc_float4_div);
PG_FUNCTION_INFO_V1(pg_enc_float4_exp);
PG_FUNCTION_INFO_V1(pg_enc_float4_mod);
PG_FUNCTION_INFO_V1(pg_enc_float4_ne);
PG_FUNCTION_INFO_V1(pg_enc_float4_eq);
PG_FUNCTION_INFO_V1(pg_enc_float4_lt);
PG_FUNCTION_INFO_V1(pg_enc_float4_le);
PG_FUNCTION_INFO_V1(pg_enc_float4_gt);
PG_FUNCTION_INFO_V1(pg_enc_float4_ge);
PG_FUNCTION_INFO_V1(pg_enc_float4_cmp);

PG_FUNCTION_INFO_V1(pg_enc_float4_max);
PG_FUNCTION_INFO_V1(pg_enc_float4_min);

PG_FUNCTION_INFO_V1(float4_to_enc_float4);
PG_FUNCTION_INFO_V1(numeric_to_enc_float4);
PG_FUNCTION_INFO_V1(double_to_enc_float4);
PG_FUNCTION_INFO_V1(int8_to_enc_float4);
PG_FUNCTION_INFO_V1(int4_to_enc_float4);
PG_FUNCTION_INFO_V1(pg_enc_float4_encrypt);
PG_FUNCTION_INFO_V1(pg_enc_float4_decrypt);
}


float4 pg_float4_in(char *num);

#ifdef _MSC_VER
#pragma warning(disable : 4756)
#endif
static inline float4
get_float4_infinity(void)
{
#ifdef INFINITY
    /* C99 standard way */
    return (float4)INFINITY;
#else
#ifdef _MSC_VER
#pragma warning(default : 4756)
#endif

    /*
     * On some platforms, HUGE_VAL is an infinity, elsewhere it's just the
     * largest normal float8.  We assume forcing an overflow will get us a
     * true infinity.
     */
    return (float4)(HUGE_VAL * HUGE_VAL);
#endif
}

static inline float4
get_float4_nan(void)
{
#ifdef NAN
    /* C99 standard way */
    return (float4)NAN;
#else
    /* Assume we can get a NAN via zero divide */
    return (float4)(0.0 / 0.0);
#endif
}


Datum pg_enc_float4_in(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    char *pSrc = PG_GETARG_CSTRING(0);
    uint64_t index = 0;
    if(pSrc[0] == FLAG_CHAR){
        index = strtoull(pSrc + 1, NULL, 10);
    }else {
        ereport(INFO, 
            (errmsg("insert temp key: %s", pSrc)));
        float src = pg_float4_in(pSrc);
        index = makeIndex(0, insertFloat(src));
    }
    PG_RETURN_DATUM(index);
}

/*
 * The function converts enc_float4 element to a string.
 * @input: enc_float4 element
 * @return: string
 */

Datum pg_enc_float4_out(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    uint64_t index = PG_GETARG_DATUM(0);
    char *str = (char *)palloc(32 * sizeof(char));
    float ans = getFloat(index);
    sprintf(str, "%f", ans);
    PG_RETURN_POINTER(str);
}

Datum numeric_to_enc_float4(PG_FUNCTION_ARGS)
{
    Numeric num = PG_GETARG_NUMERIC(0);
    char *tmp = DatumGetCString(DirectFunctionCall1(numeric_out, NumericGetDatum(num)));
    float4 src = pg_float4_in(tmp);
    uint64_t index = makeIndex(0, insertFloat(src));
    PG_RETURN_DATUM(index);
}

Datum double_to_enc_float4(PG_FUNCTION_ARGS)
{
    float8 num = PG_GETARG_FLOAT8(0);
    char *tmp = DatumGetCString(DirectFunctionCall1(float8out, Float8GetDatum(num)));
    float4 src = pg_float4_in(tmp);
    uint64_t index = makeIndex(0, insertFloat(src));
    PG_RETURN_DATUM(index);
}

Datum int8_to_enc_float4(PG_FUNCTION_ARGS)
{
    int8 num = PG_GETARG_INT64(0);
    char *tmp = DatumGetCString(DirectFunctionCall1(int8out, Int8GetDatum(num)));
    float4 src = pg_float4_in(tmp);
    uint64_t index = makeIndex(0, insertFloat(src));
    PG_RETURN_DATUM(index);
}

Datum int4_to_enc_float4(PG_FUNCTION_ARGS)
{
    int num = PG_GETARG_INT32(0);
    char *tmp = DatumGetCString(DirectFunctionCall1(int4out, Int32GetDatum(num)));
    float4 src = pg_float4_in(tmp);
    uint64_t index = makeIndex(0, insertFloat(src));
    PG_RETURN_DATUM(index);
}

Datum pg_enc_float4_encrypt(PG_FUNCTION_ARGS)
{
    PG_RETURN_DATUM(0);
}


Datum pg_enc_float4_decrypt(PG_FUNCTION_ARGS)
{
    PG_RETURN_DATUM(0);
}



Datum pg_enc_float4_sum_bulk(PG_FUNCTION_ARGS)
{
    ArrayType *v = PG_GETARG_ARRAYTYPE_P(0);
    bool isnull;
    Datum value;

    ArrayMetaState *my_extra = (ArrayMetaState *)fcinfo->flinfo->fn_extra;
    ArrayIterator array_iterator = array_create_iterator(v, 0, my_extra);
    array_iterate(array_iterator, &value, &isnull);

    uint64_t bulk_array[BULK_SIZE];
    int count = 1;
    bulk_array[0] = DatumGetUInt64(value);
    while (array_iterate(array_iterator, &value, &isnull))
    {
        if (count == BULK_SIZE)
        {
            uint64_t tmp = enc_float_sum_bulk(BULK_SIZE, bulk_array);
            bulk_array[0] = tmp;
            count = 1;
        }
        bulk_array[count] = DatumGetUInt64(value);
        count++;
    }
    uint64_t res = enc_float_sum_bulk(count, bulk_array);
    PG_RETURN_DATUM(res);
}


Datum pg_enc_float4_avg_bulk(PG_FUNCTION_ARGS)
{
    // SAME AS pg_enc_float4_avgfinal()
    ArrayType *v = PG_GETARG_ARRAYTYPE_P(0);
    bool isnull;
    Datum value;
    int ndims1 = ARR_NDIM(v); // array dimension
    int *dims1 = ARR_DIMS(v);
    int nitems = ArrayGetNItems(ndims1, dims1); // number of items in array

    ArrayMetaState *my_extra = (ArrayMetaState *)fcinfo->flinfo->fn_extra;
    ArrayIterator array_iterator = array_create_iterator(v, 0, my_extra);
    array_iterate(array_iterator, &value, &isnull);

    uint64_t bulk_array[BULK_SIZE];
    int count = 1;
    bulk_array[0] = DatumGetUInt64(value);
    while (array_iterate(array_iterator, &value, &isnull))
    {
        if (count == BULK_SIZE)
        {
            uint64_t tmp = enc_float_sum_bulk(BULK_SIZE, bulk_array);
            bulk_array[0] = tmp;
            count = 1;
        }
        bulk_array[count] = DatumGetUInt64(value);
        count++;
    }
    uint64_t sum = enc_float_sum_bulk(count, bulk_array);

    uint64_t div = makeIndex(0, insertFloat(nitems));
    uint64_t res = enc_float_div(sum, div);
    PG_RETURN_DATUM(res);

}
Datum pg_enc_float4_add(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    
    uint64_t index = enc_float_add(index1, index2);

    PG_RETURN_DATUM(index);
}

Datum pg_enc_float4_sub(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    
    uint64_t index = enc_float_sub(index1, index2);

    PG_RETURN_DATUM(index);
}

Datum pg_enc_float4_mult(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif

    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    
    uint64_t index = enc_float_mult(index1, index2);
    PG_RETURN_DATUM(index);
}

Datum pg_enc_float4_div(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    
    uint64_t index = enc_float_div(index1, index2);

    PG_RETURN_DATUM(index);
}

Datum pg_enc_float4_exp(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    
    uint64_t index = enc_float_pow(index1, index2);

    PG_RETURN_DATUM(index);
}

Datum pg_enc_float4_eq(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    int32_t cmp = enc_float_cmp(index1, index2);
    PG_RETURN_BOOL(cmp == 0);
}

Datum pg_enc_float4_ne(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    int32_t cmp = enc_float_cmp(index1, index2);
    PG_RETURN_BOOL(cmp != 0);
}

Datum pg_enc_float4_lt(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    int32_t cmp = enc_float_cmp(index1, index2);
    PG_RETURN_BOOL(cmp < 0);
}

Datum pg_enc_float4_le(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    int32_t cmp = enc_float_cmp(index1, index2);
    PG_RETURN_BOOL(cmp <= 0);
}

Datum pg_enc_float4_gt(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    int32_t cmp = enc_float_cmp(index1, index2);
    PG_RETURN_BOOL(cmp > 0);
}

Datum pg_enc_float4_ge(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    int32_t cmp = enc_float_cmp(index1, index2);
    PG_RETURN_BOOL(cmp >= 0);
}

Datum pg_enc_float4_cmp(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    int32_t cmp = enc_float_cmp(index1, index2);
    PG_RETURN_BOOL(cmp);
}

Datum pg_enc_float4_min(PG_FUNCTION_ARGS){
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);

    int32_t result = enc_float_cmp(index1, index2);
    if (result < 0) {
        PG_RETURN_DATUM(index1);
    } else {
        PG_RETURN_DATUM(index2);
    }
}

Datum pg_enc_float4_max(PG_FUNCTION_ARGS){
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);

    int32_t result = enc_int_cmp(index1, index2);
    if (result > 0) {
        PG_RETURN_DATUM(index1);
    } else {
        PG_RETURN_DATUM(index2);
    }
}




Datum pg_enc_float4_mod(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    uint64_t index = enc_float_mod(index1, index2);
    PG_RETURN_DATUM(index);
}



Datum float4_to_enc_float4(PG_FUNCTION_ARGS)
{
    PG_RETURN_DATUM(0);
}

float4 pg_float4_in(char *num)
{
    char *orig_num;
    double val;
    char *endptr;

    /*
     * endptr points to the first character _after_ the sequence we recognized
     * as a valid floating point number. orig_num points to the original input
     * string.
     */
    orig_num = num;

    /* skip leading whitespace */
    while (*num != '\0' && isspace((unsigned char)*num))
        num++;

    /*
     * Check for an empty-string input to begin with, to avoid the vagaries of
     * strtod() on different platforms.
     */
    if (*num == '\0')
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                 errmsg("invalid input syntax for type %s: \"%s\"",
                        "real", orig_num)));

    errno = 0;
    val = strtod(num, &endptr);

    /* did we not see anything that looks like a double? */
    if (endptr == num || errno != 0)
    {
        int save_errno = errno;

        /*
         * C99 requires that strtod() accept NaN, [+-]Infinity, and [+-]Inf,
         * but not all platforms support all of these (and some accept them
         * but set ERANGE anyway...)  Therefore, we check for these inputs
         * ourselves if strtod() fails.
         *
         * Note: C99 also requires hexadecimal input as well as some extended
         * forms of NaN, but we consider these forms unportable and don't try
         * to support them.  You can use 'em if your strtod() takes 'em.
         */
        if (pg_strncasecmp(num, "NaN", 3) == 0)
        {
            val = get_float4_nan();
            endptr = num + 3;
        }
        else if (pg_strncasecmp(num, "Infinity", 8) == 0)
        {
            val = get_float4_infinity();
            endptr = num + 8;
        }
        else if (pg_strncasecmp(num, "+Infinity", 9) == 0)
        {
            val = get_float4_infinity();
            endptr = num + 9;
        }
        else if (pg_strncasecmp(num, "-Infinity", 9) == 0)
        {
            val = -get_float4_infinity();
            endptr = num + 9;
        }
        else if (pg_strncasecmp(num, "inf", 3) == 0)
        {
            val = get_float4_infinity();
            endptr = num + 3;
        }
        else if (pg_strncasecmp(num, "+inf", 4) == 0)
        {
            val = get_float4_infinity();
            endptr = num + 4;
        }
        else if (pg_strncasecmp(num, "-inf", 4) == 0)
        {
            val = -get_float4_infinity();
            endptr = num + 4;
        }
        else if (save_errno == ERANGE)
        {
            /*
             * Some platforms return ERANGE for denormalized numbers (those
             * that are not zero, but are too close to zero to have full
             * precision).  We'd prefer not to throw error for that, so try to
             * detect whether it's a "real" out-of-range condition by checking
             * to see if the result is zero or huge.
             */
            if (val == 0.0 || val >= HUGE_VAL || val <= -HUGE_VAL)
                ereport(ERROR,
                        (errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
                         errmsg("\"%s\" is out of range for type real",
                                orig_num)));
        }
        else
            ereport(ERROR,
                    (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                     errmsg("invalid input syntax for type %s: \"%s\"",
                            "real", orig_num)));
    }
#ifdef HAVE_BUGGY_SOLARIS_STRTOD
    else
    {
        /*
         * Many versions of Solaris have a bug wherein strtod sets endptr to
         * point one byte beyond the end of the string when given "inf" or
         * "infinity".
         */
        if (endptr != num && endptr[-1] == '\0')
            endptr--;
    }
#endif /* HAVE_BUGGY_SOLARIS_STRTOD */

    /* skip trailing whitespace */
    while (*endptr != '\0' && isspace((unsigned char)*endptr))
        endptr++;

    /* if there is any junk left at the end of the string, bail out */
    if (*endptr != '\0')
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                 errmsg("invalid input syntax for type %s: \"%s\"",
                        "real", orig_num)));

    /*
     * if we get here, we have a legal double, still need to check to see if
     * it's a legal float4
     */
    // CHECKFLOATVAL((float4) val, isinf(val), val == 0);

    return ((float4)val);
}

