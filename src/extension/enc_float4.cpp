/*
 * a simple UDF for float4
 */
#include "extension.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
PG_FUNCTION_INFO_V1(pg_enc_float4_in);
PG_FUNCTION_INFO_V1(pg_enc_float4_out);
PG_FUNCTION_INFO_V1(pg_enc_float4_encrypt);
PG_FUNCTION_INFO_V1(pg_enc_float4_decrypt);
PG_FUNCTION_INFO_V1(pg_enc_float4_addfinal);
PG_FUNCTION_INFO_V1(pg_enc_float4_sum_bulk);
PG_FUNCTION_INFO_V1(pg_enc_float4_avgfinal);
PG_FUNCTION_INFO_V1(pg_enc_float4_avg_bulk);
PG_FUNCTION_INFO_V1(pg_enc_float4_maxfinal);
PG_FUNCTION_INFO_V1(pg_enc_float4_max_bulk);
PG_FUNCTION_INFO_V1(pg_enc_float4_minfinal);
PG_FUNCTION_INFO_V1(pg_enc_float4_min_bulk);
PG_FUNCTION_INFO_V1(pg_enc_float4_add);
PG_FUNCTION_INFO_V1(pg_enc_float4_subs);
PG_FUNCTION_INFO_V1(pg_enc_float4_mult);
PG_FUNCTION_INFO_V1(pg_enc_float4_div);
PG_FUNCTION_INFO_V1(pg_enc_float4_exp);
PG_FUNCTION_INFO_V1(pg_enc_float4_eq);
PG_FUNCTION_INFO_V1(pg_enc_float4_ne);
PG_FUNCTION_INFO_V1(pg_enc_float4_lt);
PG_FUNCTION_INFO_V1(pg_enc_float4_le);
PG_FUNCTION_INFO_V1(pg_enc_float4_gt);
PG_FUNCTION_INFO_V1(pg_enc_float4_ge);
PG_FUNCTION_INFO_V1(pg_enc_float4_cmp);
PG_FUNCTION_INFO_V1(pg_enc_float4_mod);
PG_FUNCTION_INFO_V1(float4_to_enc_float4);
PG_FUNCTION_INFO_V1(numeric_to_enc_float4);
PG_FUNCTION_INFO_V1(double_to_enc_float4);
PG_FUNCTION_INFO_V1(int8_to_enc_float4);
PG_FUNCTION_INFO_V1(int4_to_enc_float4);
#ifdef __cplusplus
}
#endif
// #define ENABLE_COUNTER
#ifdef ENABLE_COUNTER
int counter = 0;
static inline void before_invoke_function(const char *str)
{
    counter++;
    if (counter % 10000 == 0)
    {
        char ch[1000];
        sprintf(ch, "#ope invocation: %d", counter);
        print_info_str(ch);
        // print_info("#ope invocation: %d",counter );
    }
    static int add_counter = 0, cmp_counter = 0;
    if (!strcmp(str, "pg_enc_float4_add"))
    {
        add_counter++;
        if (add_counter % 10000 == 0)
        {
            char ch[1000];
            sprintf(ch, "#ope invocation: %d", counter);
            print_info_str(ch);
        }
    }
    else if (!strcmp(str, "pg_enc_float4_cmp"))
    {
        cmp_counter++;
        if (cmp_counter % 10000 == 0)
        {
            char ch[1000];
            sprintf(ch, "#ope invocation: %d", counter);
            print_info_str(ch);
        }
    }
}
#endif
static int float2bytearray(float src, uint8_t *pDst, size_t dstLen)
{
    memcpy(pDst, &src, FLOAT4_LENGTH);
    return 0;
}

static int bytearray2float(uint8_t *pSrc, float *dst, size_t srcLen)
{
    memcpy(dst, pSrc, FLOAT4_LENGTH);
    return 0;
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

/*
 * The function converts string to enc_float4. It is called by dbms every time it parses a query and finds an enc_float4 element.
 * It uses function pg_float4_in to convert it to float4 and returns an error if it can't
 * @input: string as a postgres arg
 * @return: enc_float4 element as a string
 */
Datum pg_enc_float4_in(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    char *pSrc = PG_GETARG_CSTRING(0);
    char *pDst = (char *)palloc((FLOAT4_LENGTH) * sizeof(char));
    float dst;
    dst = pg_float4_in(pSrc);
    float2bytearray(dst, pDst, FLOAT4_LENGTH);
    PG_RETURN_POINTER(pDst);
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
    char *pSrc = PG_GETARG_CSTRING(0);
    char *str = (char *)palloc(FLOAT4_LENGTH * sizeof(char));
    float ans;
    bytearray2float(pSrc, &ans, FLOAT4_LENGTH);
    sprintf(str, "%f", ans);
    PG_RETURN_POINTER(str);
}

// TODO
//  DEBUG FUNCTION
//  WILL BE DELETED IN THE PRODUCT
Datum pg_enc_float4_encrypt(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    float src = PG_GETARG_FLOAT4(0);
    char *pDst = (char *)palloc((FLOAT4_LENGTH) * sizeof(char));
    float2bytearray(src, pDst, FLOAT4_LENGTH);
    PG_RETURN_CSTRING(pDst);
}

// TODO
//  DEBUG FUNCTION
//  WILL BE DELETED IN THE PRODUCT
Datum pg_enc_float4_decrypt(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    float dst = 0;
    char *pSrc = PG_GETARG_CSTRING(0);
    bytearray2float(pSrc, &dst, FLOAT4_LENGTH);
    PG_RETURN_FLOAT4(dst);
}

/*
 * The function calculates the sum of elements from input array
 * It is called by sql aggregate command SUM, which is firstly appends needed enc_float4 elements into array and then calls this function.
 * @input: an array of enc_float4 values which should be summarize
 * @return: a enc_float4 result.
 */
Datum pg_enc_float4_addfinal(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    ArrayType *v = PG_GETARG_ARRAYTYPE_P(0);
    bool isnull;
    Datum value;
    char *pSrc1 = (char *)palloc((FLOAT4_LENGTH) * sizeof(char));
    char *pSrc2 = (char *)palloc((FLOAT4_LENGTH) * sizeof(char));
    char *pTemp = (char *)palloc((FLOAT4_LENGTH) * sizeof(char));

    ArrayMetaState *my_extra = (ArrayMetaState *)fcinfo->flinfo->fn_extra;
    ArrayIterator array_iterator = array_create_iterator(v, 0, my_extra);
    array_iterate(array_iterator, &value, &isnull);

    memcpy(pSrc1, DatumGetCString(value), FLOAT4_LENGTH);
    while (array_iterate(array_iterator, &value, &isnull))
    {
        memcpy(pTemp, DatumGetCString(value), FLOAT4_LENGTH);
        float a, b;
        bytearray2float(pTemp, &a, FLOAT4_LENGTH);
        bytearray2float(pSrc1, &b, FLOAT4_LENGTH);

        float result = a + b;

        float2bytearray(result, pSrc2, FLOAT4_LENGTH);
        memcpy(pSrc1, pSrc2, FLOAT4_LENGTH);
    }
    pfree(pTemp);
    pfree(pSrc2);

    PG_RETURN_POINTER(pSrc1);
}

Datum pg_enc_float4_sum_bulk(PG_FUNCTION_ARGS)
{
    // SAME AS pg_enc_float4_addfinal()
    ArrayType *v = PG_GETARG_ARRAYTYPE_P(0);
    bool isnull;
    Datum value;
    char *pSrc1 = (char *)palloc((FLOAT4_LENGTH) * sizeof(char));
    char *pSrc2 = (char *)palloc((FLOAT4_LENGTH) * sizeof(char));
    char *pTemp = (char *)palloc((FLOAT4_LENGTH) * sizeof(char));

    ArrayMetaState *my_extra = (ArrayMetaState *)fcinfo->flinfo->fn_extra;
    ArrayIterator array_iterator = array_create_iterator(v, 0, my_extra);
    array_iterate(array_iterator, &value, &isnull);

    memcpy(pSrc1, DatumGetCString(value), FLOAT4_LENGTH);
    while (array_iterate(array_iterator, &value, &isnull))
    {
        memcpy(pTemp, DatumGetCString(value), FLOAT4_LENGTH);
        float a, b;
        bytearray2float(pTemp, &a, FLOAT4_LENGTH);
        bytearray2float(pSrc1, &b, FLOAT4_LENGTH);

        float result = a + b;

        float2bytearray(result, pSrc2, FLOAT4_LENGTH);
        memcpy(pSrc1, pSrc2, FLOAT4_LENGTH);
    }
    pfree(pTemp);
    pfree(pSrc2);

    PG_RETURN_POINTER(pSrc1);
}

/*
 * The function computes the average of elements from array of enc_float4 elements.
 * It is called by sql aggregate command AVG, which is firstly appends needed enc_float4 elements into array and then calls this function.
 * @input: an array of enc_float4 elements
 * @return: a result (enc_float4).
 */
Datum pg_enc_float4_avgfinal(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    ArrayType *v = PG_GETARG_ARRAYTYPE_P(0);
    bool isnull;
    Datum value;
    int ndims1 = ARR_NDIM(v); // array dimension
    int *dims1 = ARR_DIMS(v);
    int nitems = ArrayGetNItems(ndims1, dims1); // number of items in array

    char *pSrc1 = (char *)palloc((FLOAT4_LENGTH) * sizeof(char));
    char *pSrc2 = (char *)palloc((FLOAT4_LENGTH) * sizeof(char));
    char *pTemp = (char *)palloc((FLOAT4_LENGTH) * sizeof(char));
    ArrayMetaState *my_extra = (ArrayMetaState *)fcinfo->flinfo->fn_extra;
    ArrayIterator array_iterator = array_create_iterator(v, 0, my_extra);
    array_iterate(array_iterator, &value, &isnull);

    memcpy(pSrc1, DatumGetCString(value), FLOAT4_LENGTH);
    float result = 0;
    while (array_iterate(array_iterator, &value, &isnull))
    {
        memcpy(pTemp, DatumGetCString(value), FLOAT4_LENGTH);
        float a, b;
        bytearray2float(pTemp, &a, FLOAT4_LENGTH);
        bytearray2float(pSrc1, &b, FLOAT4_LENGTH);

        result = a + b;

        float2bytearray(result, pSrc2, FLOAT4_LENGTH);
        memcpy(pSrc1, pSrc2, FLOAT4_LENGTH);
    }

    result /= nitems;

    float2bytearray(result, pSrc2, FLOAT4_LENGTH);
    pfree(pTemp);
    pfree(pSrc1);

    PG_RETURN_POINTER(pSrc2);
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

    char *pSrc1 = (char *)palloc((FLOAT4_LENGTH) * sizeof(char));
    char *pSrc2 = (char *)palloc((FLOAT4_LENGTH) * sizeof(char));
    char *pTemp = (char *)palloc((FLOAT4_LENGTH) * sizeof(char));
    ArrayMetaState *my_extra = (ArrayMetaState *)fcinfo->flinfo->fn_extra;
    ArrayIterator array_iterator = array_create_iterator(v, 0, my_extra);
    array_iterate(array_iterator, &value, &isnull);

    memcpy(pSrc1, DatumGetCString(value), FLOAT4_LENGTH);
    float result = 0;
    while (array_iterate(array_iterator, &value, &isnull))
    {
        memcpy(pTemp, DatumGetCString(value), FLOAT4_LENGTH);
        float a, b;
        bytearray2float(pTemp, &a, FLOAT4_LENGTH);
        bytearray2float(pSrc1, &b, FLOAT4_LENGTH);

        result = a + b;

        float2bytearray(result, pSrc2, FLOAT4_LENGTH);
        memcpy(pSrc1, pSrc2, FLOAT4_LENGTH);
    }

    result /= nitems;

    float2bytearray(result, pSrc2, FLOAT4_LENGTH);
    pfree(pTemp);
    pfree(pSrc1);

    PG_RETURN_POINTER(pSrc2);
}

/*
 * The function computes the maximal element of array of enc_float4 elements
 * It is called by sql aggregate command MAX, which first appends needed enc_float4 elements to an array and then calls this function.
 * @input: array of enc_float4 elements
 * @return: an enc_float4 result.
 */
Datum pg_enc_float4_maxfinal(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    ArrayType *v = PG_GETARG_ARRAYTYPE_P(0);
    int ans = 0;
    ArrayIterator array_iterator;
    ArrayMetaState *my_extra = (ArrayMetaState *)fcinfo->flinfo->fn_extra;
    bool isnull;
    Datum value;
    int item_count = ArrayGetNItems(ARR_NDIM(v), ARR_DIMS(v)); // number of items in array
    char *max = palloc(FLOAT4_LENGTH * sizeof(*max));
    char *value_bytes = palloc(FLOAT4_LENGTH * sizeof(*value_bytes));
    char *res = palloc(FLOAT4_LENGTH * sizeof(*res));

    array_iterator = array_create_iterator(v, 0, my_extra);
    array_iterate(array_iterator, &value, &isnull);

    memcpy(max, DatumGetCString(value), FLOAT4_LENGTH);
    while (array_iterate(array_iterator, &value, &isnull))
    {
        memcpy(value_bytes, DatumGetCString(value), FLOAT4_LENGTH);
        float a, b;
        bytearray2float(max, &a, FLOAT4_LENGTH);
        bytearray2float(value_bytes, &b, FLOAT4_LENGTH);

        ans = (a == b)  ? 0
              : (a < b) ? -1
                        : 1;

        if (ans == -1)
        {
            memcpy(max, value_bytes, FLOAT4_LENGTH);
        }
    }
    pfree(value_bytes);
    pfree(res);

    PG_RETURN_POINTER(max);
}

Datum pg_enc_float4_max_bulk(PG_FUNCTION_ARGS)
{
    // SAME AS pg_enc_float4_maxfinal()
    ArrayType *v = PG_GETARG_ARRAYTYPE_P(0);
    int ans = 0;
    ArrayIterator array_iterator;
    ArrayMetaState *my_extra = (ArrayMetaState *)fcinfo->flinfo->fn_extra;
    bool isnull;
    Datum value;
    int item_count = ArrayGetNItems(ARR_NDIM(v), ARR_DIMS(v)); // number of items in array
    char *max = palloc(FLOAT4_LENGTH * sizeof(*max));
    char *value_bytes = palloc(FLOAT4_LENGTH * sizeof(*value_bytes));
    char *res = palloc(FLOAT4_LENGTH * sizeof(*res));

    array_iterator = array_create_iterator(v, 0, my_extra);
    array_iterate(array_iterator, &value, &isnull);

    memcpy(max, DatumGetCString(value), FLOAT4_LENGTH);
    while (array_iterate(array_iterator, &value, &isnull))
    {
        memcpy(value_bytes, DatumGetCString(value), FLOAT4_LENGTH);
        float a, b;
        bytearray2float(max, &a, FLOAT4_LENGTH);
        bytearray2float(value_bytes, &b, FLOAT4_LENGTH);

        ans = (a == b)  ? 0
              : (a < b) ? -1
                        : 1;

        if (ans == -1)
        {
            memcpy(max, value_bytes, FLOAT4_LENGTH);
        }
    }
    pfree(value_bytes);
    pfree(res);

    PG_RETURN_POINTER(max);
}

/*
 * The function computes the minimal element of array of enc_float4 elements
 * It is called by sql aggregate command MIN, which first appends needed enc_float4 elements to an array and then calls this function.
 * @input: array of enc_float4 elements
 * @return: an enc_float4 result.
 */
Datum pg_enc_float4_minfinal(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    ArrayType *v = PG_GETARG_ARRAYTYPE_P(0);
    int ans = 0;
    ArrayIterator array_iterator;
    ArrayMetaState *my_extra = (ArrayMetaState *)fcinfo->flinfo->fn_extra;
    bool isnull;
    Datum value;

    int ndims1 = ARR_NDIM(v); // array dimension
    int *dims1 = ARR_DIMS(v);
    int nitems = ArrayGetNItems(ndims1, dims1); // number of items in array

    char *pSrc1 = palloc((FLOAT4_LENGTH) * sizeof(*pSrc1));
    char *pTemp = palloc((FLOAT4_LENGTH) * sizeof(*pTemp));
    char *pDst = palloc((FLOAT4_LENGTH) * sizeof(*pDst));

    array_iterator = array_create_iterator(v, 0, my_extra);
    array_iterate(array_iterator, &value, &isnull);

    memcpy(pSrc1, DatumGetCString(value), FLOAT4_LENGTH);
    while (array_iterate(array_iterator, &value, &isnull))
    {
        memcpy(pTemp, DatumGetCString(value), FLOAT4_LENGTH);
        float a, b;
        bytearray2float(pSrc1, &a, FLOAT4_LENGTH);
        bytearray2float(pTemp, &b, FLOAT4_LENGTH);

        ans = (a == b)  ? 0
              : (a < b) ? -1
                        : 1;

        if (ans == 1)
        {
            memcpy(pSrc1, pTemp, FLOAT4_LENGTH);
        }
    }
    pfree(pDst);
    pfree(pTemp);

    PG_RETURN_POINTER(pSrc1);
}

Datum pg_enc_float4_min_bulk(PG_FUNCTION_ARGS)
{
    // SAME AS pg_enc_float4_minfinal()
    ArrayType *v = PG_GETARG_ARRAYTYPE_P(0);
    int ans = 0;
    ArrayIterator array_iterator;
    ArrayMetaState *my_extra = (ArrayMetaState *)fcinfo->flinfo->fn_extra;
    bool isnull;
    Datum value;

    int ndims1 = ARR_NDIM(v); // array dimension
    int *dims1 = ARR_DIMS(v);
    int nitems = ArrayGetNItems(ndims1, dims1); // number of items in array

    char *pSrc1 = palloc((FLOAT4_LENGTH) * sizeof(*pSrc1));
    char *pTemp = palloc((FLOAT4_LENGTH) * sizeof(*pTemp));
    char *pDst = palloc((FLOAT4_LENGTH) * sizeof(*pDst));

    array_iterator = array_create_iterator(v, 0, my_extra);
    array_iterate(array_iterator, &value, &isnull);

    memcpy(pSrc1, DatumGetCString(value), FLOAT4_LENGTH);
    while (array_iterate(array_iterator, &value, &isnull))
    {
        memcpy(pTemp, DatumGetCString(value), FLOAT4_LENGTH);
        float a, b;
        bytearray2float(pSrc1, &a, FLOAT4_LENGTH);
        bytearray2float(pTemp, &b, FLOAT4_LENGTH);

        ans = (a == b)  ? 0
              : (a < b) ? -1
                        : 1;

        if (ans == 1)
        {
            memcpy(pSrc1, pTemp, FLOAT4_LENGTH);
        }
    }
    pfree(pDst);
    pfree(pTemp);

    PG_RETURN_POINTER(pSrc1);
}
/*
 * The function calculates the sum of two enc_float4 values. It is called by binary operator '+' defined in sql extension.
 * @input: two enc_float4 values
 * @return: sum of input values
 */

Datum pg_enc_float4_add(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    float *c1 = (float *)PG_GETARG_CSTRING(0);
    float *c2 = (float *)PG_GETARG_CSTRING(1);
    float *pDst = (float *)palloc((FLOAT4_LENGTH) * sizeof(char));

    *pDst = *c1 + *c2;

    PG_RETURN_POINTER(pDst);
}

/*
 * The function calculates the subtraction of two enc_float4 values. It is called by binary operator '-' defined in sql extension.
 * @input: two enc_float4 values
 * @return: result of input values .
 */
Datum pg_enc_float4_subs(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    char *c1 = PG_GETARG_CSTRING(0);
    char *c2 = PG_GETARG_CSTRING(1);
    char *pDst = (char *)palloc((FLOAT4_LENGTH) * sizeof(char));
    float a, b;
    bytearray2float(c1, &a, FLOAT4_LENGTH);
    bytearray2float(c2, &b, FLOAT4_LENGTH);

    float result = a - b;

    float2bytearray(result, pDst, FLOAT4_LENGTH);
    PG_RETURN_POINTER(pDst);
}

/*
 * The function calculates the product of two enc_float4 values. It is called by binary operator '*' defined in sql extension.
 * @input: two enc_float4 values
 * @return: an enc_float4 result of input values .
 */
Datum pg_enc_float4_mult(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    char *c1 = PG_GETARG_CSTRING(0);
    char *c2 = PG_GETARG_CSTRING(1);
    char *pDst = (char *)palloc((FLOAT4_LENGTH) * sizeof(char));
    float a, b;
    bytearray2float(c1, &a, FLOAT4_LENGTH);
    bytearray2float(c2, &b, FLOAT4_LENGTH);

    float result = a * b;

    float2bytearray(result, pDst, FLOAT4_LENGTH);
    PG_RETURN_POINTER(pDst);
}

/*
 * The function calculates the division of two enc_float4 values. It is called by binary operator '/' defined in sql extension.
 * @input: two enc_float4 values
 * @return: an enc_float4 result of input values .
 */

Datum pg_enc_float4_div(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    char *c1 = PG_GETARG_CSTRING(0);
    char *c2 = PG_GETARG_CSTRING(1);
    char *pDst = (char *)palloc((FLOAT4_LENGTH) * sizeof(char));
    float a, b;
    bytearray2float(c1, &a, FLOAT4_LENGTH);
    bytearray2float(c2, &b, FLOAT4_LENGTH);

    float result = a / b;

    float2bytearray(result, pDst, FLOAT4_LENGTH);
    PG_RETURN_POINTER(pDst);
}

/*
 * The function calculates the first input enc_float4 value to the power of the second input enc_float4 value.
 * It is called by binary operator '^' defined in sql extension.
 * @input: two enc_float4 values
 * @return: an enc_float4 result of input values .
 */
Datum pg_enc_float4_exp(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    char *c1 = PG_GETARG_CSTRING(0);
    char *c2 = PG_GETARG_CSTRING(1);
    char *pDst = (char *)palloc((FLOAT4_LENGTH) * sizeof(char));
    float a, b;
    bytearray2float(c1, &a, FLOAT4_LENGTH);
    bytearray2float(c2, &b, FLOAT4_LENGTH);

    float result = pow(a, b);

    float2bytearray(result, pDst, FLOAT4_LENGTH);
    PG_RETURN_POINTER(pDst);
}

/*
 * The function checks if the first input enc_float4 is equal to the second one.
 * It is called by binary operator '=' defined in sql extension.
 * @input: two enc_float4 values
 * @return: true, if the first float is equal to the second one.
 *       false, otherwise
 */
Datum pg_enc_float4_eq(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    float *c1 = (float *)PG_GETARG_CSTRING(0);
    float *c2 = (float *)PG_GETARG_CSTRING(1);
    int ans = 0;
    float a, b;
    a = *c1, b = *c2;

    int cmp = (a == b) ? true : false;

    PG_RETURN_BOOL(cmp);
}

/*
 * The function checks if the first input enc_float4 is not equal to the second one.
 * It is called by binary operator '!=' defined in sql extension.
 * @input: two enc_float4 values
 * @return: true, if the first float is not equal to the second one.
 *       false, otherwise
 */
Datum pg_enc_float4_ne(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    float *c1 = (float *)PG_GETARG_CSTRING(0);
    float *c2 = (float *)PG_GETARG_CSTRING(1);
    int ans = 0;
    float a, b;
    a = *c1, b = *c2;

    int cmp = (a != b) ? true : false;

    PG_RETURN_BOOL(cmp);
}

/*
 * The function checks if the first input enc_float4 is less than the second one.
 * It is called by binary operator '<' defined in sql extension.
 * @input: two enc_float4 values
 * @return: true, if the first decrypted float is less the the second one.
 *       false, otherwise
 */
Datum pg_enc_float4_lt(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    float *c1 = (float *)PG_GETARG_CSTRING(0);
    float *c2 = (float *)PG_GETARG_CSTRING(1);
    int ans = 0;
    float a, b;
    a = *c1, b = *c2;
    int cmp = (a < b) ? true : false;

    PG_RETURN_BOOL(cmp);
}

/*
 * The function checks if the first input enc_float4 is less or equal than the second one.
 * It is called by binary operator '<=' defined in sql extension.
 * @input: two enc_float4 values
 * @return: true, if the first enc_float4 is less or equal than the second one.
 *       false, otherwise
 */
Datum pg_enc_float4_le(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    float *c1 = (float *)PG_GETARG_CSTRING(0);
    float *c2 = (float *)PG_GETARG_CSTRING(1);
    int ans = 0;
    float a, b;
    a = *c1, b = *c2;
    int cmp = (a <= b) ? true : false;

    PG_RETURN_BOOL(cmp);
}

/*
 * The function checks if the first input enc_float4 is greater than the second one.
 * It is called by binary operator '>' defined in sql extension.
 * @input: two enc_float4 values
 * @return: true, if the first float is greater than the second one.
 *          false, otherwise
 */
Datum pg_enc_float4_gt(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    float *c1 = (float *)PG_GETARG_CSTRING(0);
    float *c2 = (float *)PG_GETARG_CSTRING(1);
    int ans = 0;
    float a, b;
    a = *c1, b = *c2;
    int cmp = (a > b) ? true : false;

    PG_RETURN_BOOL(cmp);
}

/*
 * The function checks if the first input enc_float4 is greater or equal than the second one.
 * It is called by binary operator '>=' defined in sql extension.
 * @input: two enc_float4 values
 * @return: true, if the first float is greater or equal than the second one.
 *          false, otherwise
 */
Datum pg_enc_float4_ge(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    float *c1 = (float *)PG_GETARG_CSTRING(0);
    float *c2 = (float *)PG_GETARG_CSTRING(1);
    int ans = 0;
    float a, b;
    a = *c1, b = *c2;

    int cmp = (a >= b) ? true : false;

    PG_RETURN_BOOL(cmp);
}

/*
 * The function compares two enc_float4 values. It is called mostly during index building.
 * @input: two enc_float4 values
 * @return: -1, 0 ,1
 */
Datum pg_enc_float4_cmp(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    float *c1 = (float *)PG_GETARG_CSTRING(0);
    float *c2 = (float *)PG_GETARG_CSTRING(1);
    int ans = 0;
    float a, b;
    a = *c1, b = *c2;
    ans = (a == b)  ? 0
          : (a < b) ? -1
                    : 1;

    PG_RETURN_INT32(ans);
}

/*
 * The function calculates the first input enc_float4 value by module the second input enc_float4 value.
 * It is called by binary operator '%' defined in sql extension.
 * @input: two enc_float4 values
 * @return: result of input values .
 */
Datum pg_enc_float4_mod(PG_FUNCTION_ARGS)
{
#ifdef ENABLE_COUNTER
    before_invoke_function(__func__);
#endif
    char *c1 = PG_GETARG_CSTRING(0);
    char *c2 = PG_GETARG_CSTRING(1);
    char *pDst = (char *)palloc((FLOAT4_LENGTH) * sizeof(char));
    float a, b;
    bytearray2float(c1, &a, FLOAT4_LENGTH);
    bytearray2float(c2, &b, FLOAT4_LENGTH);

    float result = (int)a % (int)b;

    float2bytearray(result, pDst, FLOAT4_LENGTH);
    PG_RETURN_POINTER(pDst);
}

/*
 * The function converts a float to enc_float4 value. This function is called by sql function CAST.
 * @input: float4
 * @return: an encrypted result.
 */
Datum float4_to_enc_float4(PG_FUNCTION_ARGS)
{
    float src = PG_GETARG_FLOAT4(0);
    char *pDst = (char *)palloc((FLOAT4_LENGTH) * sizeof(char));
    float2bytearray(src, pDst, FLOAT4_LENGTH);
    PG_RETURN_POINTER((const char *)pDst);
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

/*
 * The function converts a numeric datatype(postgres variable datatype can be any of int2, int4, int8, float4, float8) to enc_float4 value.
 * This function is called by sql function CAST. It uses function pg_float4_in to convert it to float4 and return an error if it can't
 * @input: float4
 * @return: an enc_float4 result.
 */
Datum numeric_to_enc_float4(PG_FUNCTION_ARGS)
{
    Numeric num = PG_GETARG_NUMERIC(0);
    char *pDst = (char *)palloc((FLOAT4_LENGTH) * sizeof(char));
    float4 src;
    char *tmp = DatumGetCString(DirectFunctionCall1(numeric_out, NumericGetDatum(num)));
    src = pg_float4_in(tmp);
    float2bytearray(src, pDst, FLOAT4_LENGTH);
    pfree(tmp);
    PG_RETURN_POINTER(pDst);
}

/*
 * The function converts a double precision datatype to enc_float4 value.
 * This function is called by sql function CAST. It uses function pg_float4_in to convert it to float4 and return an error if it can't
 * @input: float8
 * @return: an enc_float4 result.
 */
Datum double_to_enc_float4(PG_FUNCTION_ARGS)
{
    float8 num = PG_GETARG_FLOAT8(0);
    char *pDst = (char *)palloc((FLOAT4_LENGTH) * sizeof(char));
    int ans;
    float4 src;
    char *tmp = DatumGetCString(DirectFunctionCall1(float8out, Float8GetDatum(num)));
    src = pg_float4_in(tmp);
    float2bytearray(src, pDst, FLOAT4_LENGTH);
    pfree(tmp);
    PG_RETURN_POINTER(pDst);
}

/*
 * The function converts a bigint (int8) datatype to enc_float4 value.
 * This function is called by sql function CAST. It uses function pg_float4_in to convert it to float4 and return an error if it can't
 * @input: int8
 * @return: an enc_float4 result.
 */
Datum int8_to_enc_float4(PG_FUNCTION_ARGS)
{
    int8 num = PG_GETARG_INT64(0);
    char *pDst = (char *)palloc((FLOAT4_LENGTH) * sizeof(char));
    int ans;
    float4 src;
    char *tmp = DatumGetCString(DirectFunctionCall1(int8out, Int8GetDatum(num)));
    src = pg_float4_in(tmp);
    float2bytearray(src, pDst, FLOAT4_LENGTH);
    pfree(tmp);
    PG_RETURN_POINTER(pDst);
}

/*
 * The function converts a int (int4) datatype to enc_float4 value.
 * This function is called by sql function CAST. It uses function pg_float4_in to convert it to float4 and return an error if it can't
 * @input: int4
 * @return: an enc_float4 result.
 */
Datum int4_to_enc_float4(PG_FUNCTION_ARGS)
{
    int num = PG_GETARG_INT32(0);
    char *pDst = (char *)palloc((FLOAT4_LENGTH) * sizeof(char));
    int ans;
    float4 src;
    char *tmp = DatumGetCString(DirectFunctionCall1(int4out, Int32GetDatum(num)));
    src = pg_float4_in(tmp);
    float2bytearray(src, pDst, FLOAT4_LENGTH);
    pfree(tmp);
    PG_RETURN_POINTER(pDst);
}
