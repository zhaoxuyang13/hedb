/*
 * a simple UDF for int4
 */
#include <extension.h>

#define PRINT_ALL_FUNC 0

static inline void int2bytearray(int src, uint8_t* pDst, size_t dstLen)
{
    memcpy(pDst, &src, INT32_LENGTH);
}

static inline void bytearray2int(uint8_t* pSrc, int* dst, size_t srcLen)
{
    memcpy(dst, pSrc, INT32_LENGTH);
}

/*
 * The function converts string to enc_int4. It is called by dbms every time it parses a query and finds an enc_int4 element.
 * If flag debugMode is true it tries to convert input to int4 and encrypt it
 * @input: string as a postgres arg
 * @return: enc_int4 element as a string
 */
PG_FUNCTION_INFO_V1(pg_enc_int4_in);
Datum
    pg_enc_int4_in(PG_FUNCTION_ARGS)
{
#if PRINT_ALL_FUNC
    static printed = false;
    if(!printed) {
        ereport(LOG, (errmsg("pg_enc_int4_in here!")));
        printed = true;
    }
#endif
    // ereport(INFO, (errmsg("pg_enc_int4_in here!")));
    char* pSrc = PG_GETARG_CSTRING(0);
    char* pDst = (char*)palloc((INT32_LENGTH) * sizeof(char));
    int dst = pg_atoi(pSrc, INT32_LENGTH, '\0');;
    int2bytearray(dst, pDst, INT32_LENGTH);
    PG_RETURN_POINTER(pDst);
}

/*
 * The function converts enc_int4 element to a string. If flag debugMode is true it decrypts the string and return unencrypted result.
 * @input: enc_int4 element
 * @return: string
 */
PG_FUNCTION_INFO_V1(pg_enc_int4_out);
Datum
    pg_enc_int4_out(PG_FUNCTION_ARGS)
{
#if PRINT_ALL_FUNC
    static printed1 = false;
    if(!printed1) {
        ereport(INFO, (errmsg("pg_enc_int4_out here!")));
        printed1 = true;
    }
#endif
    // ereport(INFO, (errmsg("pg_enc_int4_out here!")));
    char* pSrc = PG_GETARG_CSTRING(0);
    char* str = (char*)palloc(INT32_LENGTH * sizeof(char));
    int ans;
    bytearray2int(pSrc, &ans, INT32_LENGTH);
    sprintf(str, "%d", ans);
    PG_RETURN_POINTER(str);
}

/*
 * The function calculates the sum of two enc_int4 values. It is called by binary operator '+' defined in sql extension.
 * @input: two enc_int4 values
 * @return: enc_int4 sum of input values
*/
PG_FUNCTION_INFO_V1(pg_enc_int4_add);
Datum
    pg_enc_int4_add(PG_FUNCTION_ARGS)
{
#if PRINT_ALL_FUNC
    static printed2 = false;
    if(!printed2) {
        ereport(LOG, (errmsg("pg_enc_int4_add here!")));
        printed2 = true;
    }
#endif
    int* c1 = PG_GETARG_ENCINT(0);
    int* c2 = PG_GETARG_ENCINT(1);
    int* pDst = (int*)palloc(sizeof(int));

    *pDst = *c1 + *c2; 

    PG_RETURN_POINTER(pDst);
}

/*
 * The function calculates the subtraction of two enc_int4 values. It is called by binary operator '-' defined in sql extension.
 * @input: two enc_int4 values
 * @return: an enc_int4 result of input values .
 */
PG_FUNCTION_INFO_V1(pg_enc_int4_sub);
Datum
    pg_enc_int4_sub(PG_FUNCTION_ARGS)
{
#if PRINT_ALL_FUNC
    static printed3 = false;
    if(!printed3) {
        ereport(LOG, (errmsg("pg_enc_int4_sub here!")));
        printed3 = true;
    }
#endif
    char* c1 = PG_GETARG_CSTRING(0);
    char* c2 = PG_GETARG_CSTRING(1);
    char* pDst = (char*)palloc((INT32_LENGTH) * sizeof(char));

    int32_t a, b, sub;
    bytearray2int(c1, &a, INT32_LENGTH);
    bytearray2int(c2, &b, INT32_LENGTH);

    sub = a - b;

    int2bytearray((int32_t)sub, pDst, INT32_LENGTH);
    PG_RETURN_POINTER(pDst);
}

/*
 * The function calculates the product of two enc_int4 values. It is called by binary operator '*' defined in sql extension.
 * @input: two enc_int4 values
 * @return: an enc_int4 result of input values .
 */
PG_FUNCTION_INFO_V1(pg_enc_int4_mult);
Datum
    pg_enc_int4_mult(PG_FUNCTION_ARGS)
{
#if PRINT_ALL_FUNC
    static printed = false;
    if(!printed) {
        ereport(LOG, (errmsg("pg_enc_int4_mult here!")));
        printed = true;
    }
#endif
    char* c1 = PG_GETARG_CSTRING(0);
    char* c2 = PG_GETARG_CSTRING(1);
    char* pDst = (char*)palloc((INT32_LENGTH) * sizeof(char));

    int32_t a, b, mult;
    bytearray2int(c1, &a, INT32_LENGTH);
    bytearray2int(c2, &b, INT32_LENGTH);

    mult = a * b;

    int2bytearray((int32_t)mult, pDst, INT32_LENGTH);
    PG_RETURN_POINTER(pDst);
}

/*
 * The function calculates the division of two enc_int4 values. It is called by binary operator '/' defined in sql extension.
 * @input: two enc_int4 values
 * @return: an enc_int4 result of input values .
 */
PG_FUNCTION_INFO_V1(pg_enc_int4_div);
Datum
    pg_enc_int4_div(PG_FUNCTION_ARGS)
{
#if PRINT_ALL_FUNC
    static printed = false;
    if(!printed) {
        ereport(LOG, (errmsg("pg_enc_int4_div here!")));
        printed = true;
    }
#endif
    char* c1 = PG_GETARG_CSTRING(0);
    char* c2 = PG_GETARG_CSTRING(1);
    char* pDst = (char*)palloc((INT32_LENGTH) * sizeof(char));

    int32_t a, b, div;
    bytearray2int(c1, &a, INT32_LENGTH);
    bytearray2int(c2, &b, INT32_LENGTH);

    div = a / b;

    int2bytearray((int32_t)div, pDst, INT32_LENGTH);
    PG_RETURN_POINTER(pDst);
}

/*
 * The function calculates the first input enc_int4 value to the power of the second input enc_int4 value.
 * It is called by binary operator '^' defined in sql extension.
 * @input: two enc_int4 values
 * @return: an result of input values .
 */
PG_FUNCTION_INFO_V1(pg_enc_int4_pow);
Datum
    pg_enc_int4_pow(PG_FUNCTION_ARGS)
{
#if PRINT_ALL_FUNC
    static printed = false;
    if(!printed) {
        ereport(LOG, (errmsg("pg_enc_int4_pow here!")));
        printed = true;
    }
#endif
    char* c1 = PG_GETARG_CSTRING(0);
    char* c2 = PG_GETARG_CSTRING(1);
    char* pDst = (char*)palloc((INT32_LENGTH) * sizeof(char));

    int32_t a, b, _pow;
    bytearray2int(c1, &a, INT32_LENGTH);
    bytearray2int(c2, &b, INT32_LENGTH);

    _pow = (int32_t)pow((double)a, (double)b);

    int2bytearray((int32_t)_pow, pDst, INT32_LENGTH);
    PG_RETURN_POINTER(pDst);
}

/*
 * The function calculates the first input enc_int4 value by module the second input enc_int4 value.
 * It is called by binary operator '%' defined in sql extension.
 * @input: two enc_int4 values
 * @return: an result of input values .
 */
PG_FUNCTION_INFO_V1(pg_enc_int4_mod);
Datum
    pg_enc_int4_mod(PG_FUNCTION_ARGS)
{
#if PRINT_ALL_FUNC
    static printed = false;
    if(!printed) {
        ereport(LOG, (errmsg("pg_enc_int4_mod here!")));
        printed = true;
    }
#endif
    char* c1 = PG_GETARG_CSTRING(0);
    char* c2 = PG_GETARG_CSTRING(1);
    char* pDst = (char*)palloc((INT32_LENGTH) * sizeof(char));

    int32_t a, b, mod;
    bytearray2int(c1, &a, INT32_LENGTH);
    bytearray2int(c2, &b, INT32_LENGTH);

    mod = a % b;

    int2bytearray((int32_t)mod, pDst, INT32_LENGTH);
    PG_RETURN_POINTER(pDst);
}

/*
 * The function compares two enc_int4 values. It is called mostly during index building.
 * @input: two enc_int4 values
 * @return: -1, 0 ,1
 */
PG_FUNCTION_INFO_V1(pg_enc_int4_cmp);
Datum
    pg_enc_int4_cmp(PG_FUNCTION_ARGS)
{
#if PRINT_ALL_FUNC
    static printed4 = false;
    if(!printed4) {
        ereport(LOG, (errmsg("pg_enc_int4_cmp here!")));
        printed4 = true;
    }
#endif
    int* c1 = PG_GETARG_ENCINT(0);
    int* c2 = PG_GETARG_ENCINT(1);

    int a = *c1, b= *c2;

    PG_RETURN_INT32(a - b);
}

/*
 * The function checks if the first input enc_int4 is equal to the second one.
 * It is called by binary operator '=' defined in sql extension.
 * @input: two enc_int4 values
 * @return: true, if the first integer is equal to the second one.
 *       false, otherwise
*/
PG_FUNCTION_INFO_V1(pg_enc_int4_eq);
Datum
    pg_enc_int4_eq(PG_FUNCTION_ARGS)
{
#if PRINT_ALL_FUNC
    static printed = false;
    if(!printed) {
        ereport(LOG, (errmsg("pg_enc_int4_eq here!")));
        printed = true;
    }
#endif
    char* c1 = PG_GETARG_CSTRING(0);
    char* c2 = PG_GETARG_CSTRING(1);

    int32_t a, b;
    bytearray2int(c1, &a, INT32_LENGTH);
    bytearray2int(c2, &b, INT32_LENGTH);
    PG_RETURN_BOOL(a == b);
}

/*
 * The function checks if the first input enc_int4 is not equal to the second one.
 * It is called by binary operator '!=' defined in sql extension.
 * @input: two enc_int4 values
 * @return: true, if the first integer is not equal to the second one.
 *       false, otherwise
 */
PG_FUNCTION_INFO_V1(pg_enc_int4_ne);
Datum
    pg_enc_int4_ne(PG_FUNCTION_ARGS)
{
#if PRINT_ALL_FUNC
    static printed = false;
    if(!printed) {
        ereport(LOG, (errmsg("pg_enc_int4_ne here!")));
        printed = true;
    }
#endif
    char* c1 = PG_GETARG_CSTRING(0);
    char* c2 = PG_GETARG_CSTRING(1);

    int32_t a, b;
    bytearray2int(c1, &a, INT32_LENGTH);
    bytearray2int(c2, &b, INT32_LENGTH);
    PG_RETURN_BOOL(a != b);
}

/*
 * The function checks if the first input enc_int4 is less than the second one.
 * It is called by binary operator '<' defined in sql extension.
 * @input: two enc_int4 values
 * @return: true, if the first integer is less the the second one.
 *       false, otherwise
 */
PG_FUNCTION_INFO_V1(pg_enc_int4_lt);
Datum
    pg_enc_int4_lt(PG_FUNCTION_ARGS)
{
    char* c1 = PG_GETARG_CSTRING(0);
    char* c2 = PG_GETARG_CSTRING(1);

    int32_t a, b;
    bytearray2int(c1, &a, INT32_LENGTH);
    bytearray2int(c2, &b, INT32_LENGTH);
    PG_RETURN_BOOL(a < b);
}

/*
 * The function checks if the first input enc_int4 is less or equal than the second one.
 * It is called by binary operator '<=' defined in sql extension.
 * @input: two enc_int4 values
 * @return: true, if the first integer is less or equal than the second one.
 *       false, otherwise
 */
PG_FUNCTION_INFO_V1(pg_enc_int4_le);
Datum
    pg_enc_int4_le(PG_FUNCTION_ARGS)
{
    char* c1 = PG_GETARG_CSTRING(0);
    char* c2 = PG_GETARG_CSTRING(1);

    int32_t a, b;
    bytearray2int(c1, &a, INT32_LENGTH);
    bytearray2int(c2, &b, INT32_LENGTH);

    PG_RETURN_BOOL(a <= b);
}

/*
 * The function checks if the first input enc_int4 is greater than the second one.
 * It is called by binary operator '>' defined in sql extension.
 * @input: two enc_int4 values
 * @return: true, if the first integer is greater than the second one.
 *          false, otherwise
 */
PG_FUNCTION_INFO_V1(pg_enc_int4_gt);
Datum
    pg_enc_int4_gt(PG_FUNCTION_ARGS)
{
#if PRINT_ALL_FUNC
    static printed = false;
    if(!printed) {
        ereport(LOG, (errmsg("pg_enc_int4_gt here!")));
        printed = true;
    }
#endif
    char* c1 = PG_GETARG_CSTRING(0);
    char* c2 = PG_GETARG_CSTRING(1);

    int32_t a, b;
    bytearray2int(c1, &a, INT32_LENGTH);
    bytearray2int(c2, &b, INT32_LENGTH);

    PG_RETURN_BOOL(a > b);
}

/*
 * The function checks if the first input enc_int4 is greater or equal than the second one.
 * It is called by binary operator '>=' defined in sql extension.
 * @input: two enc_int4 values
 * @return: true, if the first integer is greater or equal than the second one.
 *          false, otherwise
 */
PG_FUNCTION_INFO_V1(pg_enc_int4_ge);
Datum
    pg_enc_int4_ge(PG_FUNCTION_ARGS)
{
#if PRINT_ALL_FUNC
    static printed = false;
    if(!printed) {
        ereport(LOG, (errmsg("pg_enc_int4_ge here!")));
        printed = true;
    }
#endif
    char* c1 = PG_GETARG_CSTRING(0);
    char* c2 = PG_GETARG_CSTRING(1);

    int32_t a, b;
    bytearray2int(c1, &a, INT32_LENGTH);
    bytearray2int(c2, &b, INT32_LENGTH);

    PG_RETURN_BOOL(a >= b);
}

//TODO
// DEBUG FUNCTION
// WILL BE DELETED IN THE PRODUCT
PG_FUNCTION_INFO_V1(pg_enc_int4_encrypt);
Datum
    pg_enc_int4_encrypt(PG_FUNCTION_ARGS)
{
#if PRINT_ALL_FUNC
    static printed5 = false;
    if(!printed5) {
        ereport(LOG, (errmsg("pg_enc_int4_encrypt here!")));
        printed5 = true;
    }
#endif
    // ereport(INFO, (errmsg("pg_enc_int4_encrypt here!")));
    int src = PG_GETARG_INT32(0);
    char* pDst = (char*)palloc((INT32_LENGTH) * sizeof(char));
    int2bytearray(src, pDst, INT32_LENGTH);
    PG_RETURN_CSTRING(pDst);
}

//TODO
// DEBUG FUNCTION
// WILL BE DELETED IN THE PRODUCT
PG_FUNCTION_INFO_V1(pg_enc_int4_decrypt);
Datum
    pg_enc_int4_decrypt(PG_FUNCTION_ARGS)
{
#if PRINT_ALL_FUNC
    static printed6 = false;
    if(!printed6) {
        ereport(LOG, (errmsg("pg_enc_int4_decrypt here!")));
        printed6 = true;
    }
#endif
    // ereport(INFO, (errmsg("pg_enc_int4_decrypt here!")));
    int dst = 0;
    char* pSrc = PG_GETARG_CSTRING(0);
    bytearray2int(pSrc, &dst, INT32_LENGTH);
    PG_RETURN_INT32(dst);
}

/*
 * The function calculates the sum of elements from input array
 * It is called by sql aggregate command SUM, which is firstly appends needed enc_int4 elements into array and then calls this function.
 * @input: an array of enc_int4 values which should be summarize
 * @return: an enc_int4 result.
 */
PG_FUNCTION_INFO_V1(pg_enc_int4_addfinal);
Datum
    pg_enc_int4_addfinal(PG_FUNCTION_ARGS)
{
#if PRINT_ALL_FUNC
    static printed = false;
    if(!printed) {
        ereport(LOG, (errmsg("pg_enc_int4_addfinal here!")));
        printed = true;
    }
#endif
    // ereport(INFO, (errmsg("pg_enc_int4_addfinal begin")));
    ArrayType* v = PG_GETARG_ARRAYTYPE_P(0);
    ArrayIterator array_iterator;
    ArrayMetaState* my_extra = (ArrayMetaState*)fcinfo->flinfo->fn_extra;
    bool isnull;
    Datum value;
    char* pSrc1 = (char*)palloc((INT32_LENGTH) * sizeof(char));
    char* pSrc2 = (char*)palloc((INT32_LENGTH) * sizeof(char));
    char* pTemp = (char*)palloc((INT32_LENGTH) * sizeof(char));

    array_iterator = array_create_iterator(v, 0, my_extra);
    array_iterate(array_iterator, &value, &isnull);

    // get first arg
    memcpy(pSrc1, DatumGetCString(value), INT32_LENGTH);

    int sum_all = 0, a, b;
    bytearray2int(pSrc1, &a, INT32_LENGTH);
    sum_all = a;

    while (array_iterate(array_iterator, &value, &isnull))
    {
        // get next arg
        memcpy(pTemp, DatumGetCString(value), INT32_LENGTH);
        bytearray2int(pTemp, &b, INT32_LENGTH);
        sum_all += b;
    }
    int2bytearray(sum_all, pSrc1, INT32_LENGTH);
    pfree(pTemp);
    pfree(pSrc2);

    PG_RETURN_POINTER(pSrc1);
}

PG_FUNCTION_INFO_V1(pg_enc_int4_sum_bulk);
Datum
    pg_enc_int4_sum_bulk(PG_FUNCTION_ARGS)
{
#if PRINT_ALL_FUNC
    static printed = false;
    if(!printed) {
        ereport(LOG, (errmsg("pg_enc_int4_sum_bulk here!")));
        printed = true;
    }
#endif
    // SAME AS pg_enc_int4_addfinal()
    ArrayType* v = PG_GETARG_ARRAYTYPE_P(0);
    ArrayIterator array_iterator;
    ArrayMetaState* my_extra = (ArrayMetaState*)fcinfo->flinfo->fn_extra;
    bool isnull;
    Datum value;
    char* pSrc1 = (char*)palloc((INT32_LENGTH) * sizeof(char));
    char* pSrc2 = (char*)palloc((INT32_LENGTH) * sizeof(char));
    char* pTemp = (char*)palloc((INT32_LENGTH) * sizeof(char));

    array_iterator = array_create_iterator(v, 0, my_extra);
    array_iterate(array_iterator, &value, &isnull);

    // get first arg
    memcpy(pSrc1, DatumGetCString(value), INT32_LENGTH);

    int sum_all = 0, a, b;
    bytearray2int(pSrc1, &a, INT32_LENGTH);
    sum_all = a;

    while (array_iterate(array_iterator, &value, &isnull))
    {
        // get next arg
        memcpy(pTemp, DatumGetCString(value), INT32_LENGTH);
        bytearray2int(pTemp, &b, INT32_LENGTH);
        sum_all += b;
    }
    int2bytearray(sum_all, pSrc1, INT32_LENGTH);
    pfree(pTemp);
    pfree(pSrc2);

    PG_RETURN_POINTER(pSrc1);
}

/*
 * The function computes the average of elements from array of enc_int4 elements.
 * It is called by sql aggregate command AVG, which is firstly appends needed enc_int4 elements into array and then calls this function.
 * @input: an array of enc_int4 elements
 * @return: an enc_int4 result.
 */
PG_FUNCTION_INFO_V1(pg_enc_int4_avgfinal);
Datum
    pg_enc_int4_avgfinal(PG_FUNCTION_ARGS)
{
#if PRINT_ALL_FUNC
    static printed = false;
    if(!printed) {
        ereport(LOG, (errmsg("pg_enc_int4_avgfinal here!")));
        printed = true;
    }
#endif
    ArrayType* v = PG_GETARG_ARRAYTYPE_P(0);
    ArrayIterator array_iterator;
    ArrayMetaState* my_extra = (ArrayMetaState*)fcinfo->flinfo->fn_extra;
    bool isnull;
    Datum value;
    int ndims1 = ARR_NDIM(v); //array dimension
    int* dims1 = ARR_DIMS(v);
    int nitems = ArrayGetNItems(ndims1, dims1); //number of items in array
    char* pSrc1 = (char*)palloc((INT32_LENGTH) * sizeof(char));
    char* pSrc2 = (char*)palloc((INT32_LENGTH) * sizeof(char));
    char* pTemp = (char*)palloc((INT32_LENGTH) * sizeof(char));

    array_iterator = array_create_iterator(v, 0, my_extra);
    array_iterate(array_iterator, &value, &isnull);

    // get arg1
    memcpy(pSrc1, DatumGetCString(value), INT32_LENGTH);

    int sum_all = 0, a, b;
    bytearray2int(pSrc1, &a, INT32_LENGTH);
    sum_all = a;

    while (array_iterate(array_iterator, &value, &isnull))
    {
        // get next arg
        memcpy(pTemp, DatumGetCString(value), INT32_LENGTH);
        bytearray2int(pTemp, &b, INT32_LENGTH);

        sum_all += b;
    }
    sum_all /= nitems; // div
    int2bytearray((int32_t)sum_all, pSrc2, INT32_LENGTH);

    pfree(pTemp);
    pfree(pSrc1);

    PG_RETURN_POINTER(pSrc2);
}

PG_FUNCTION_INFO_V1(pg_enc_int4_avg_bulk);
Datum
    pg_enc_int4_avg_bulk(PG_FUNCTION_ARGS)
{
#if PRINT_ALL_FUNC
    static printed = false;
    if(!printed) {
        ereport(LOG, (errmsg("pg_enc_int4_avg_bulk here!")));
        printed = true;
    }
#endif
    // SAME AS pg_enc_int4_avgfinal()
    ArrayType* v = PG_GETARG_ARRAYTYPE_P(0);
    ArrayIterator array_iterator;
    ArrayMetaState* my_extra = (ArrayMetaState*)fcinfo->flinfo->fn_extra;
    bool isnull;
    Datum value;
    int ndims1 = ARR_NDIM(v); //array dimension
    int* dims1 = ARR_DIMS(v);
    int nitems = ArrayGetNItems(ndims1, dims1); //number of items in array
    char* pSrc1 = (char*)palloc((INT32_LENGTH) * sizeof(char));
    char* pSrc2 = (char*)palloc((INT32_LENGTH) * sizeof(char));
    char* pTemp = (char*)palloc((INT32_LENGTH) * sizeof(char));

    array_iterator = array_create_iterator(v, 0, my_extra);
    array_iterate(array_iterator, &value, &isnull);

    // get arg1
    memcpy(pSrc1, DatumGetCString(value), INT32_LENGTH);

    int sum_all = 0, a, b;
    bytearray2int(pSrc1, &a, INT32_LENGTH);
    sum_all = a;

    while (array_iterate(array_iterator, &value, &isnull))
    {
        // get next arg
        memcpy(pTemp, DatumGetCString(value), INT32_LENGTH);
        bytearray2int(pTemp, &b, INT32_LENGTH);

        sum_all += b;
    }
    sum_all /= nitems; // div
    int2bytearray((int32_t)sum_all, pSrc2, INT32_LENGTH);

    pfree(pTemp);
    pfree(pSrc1);

    PG_RETURN_POINTER(pSrc2);
}

PG_FUNCTION_INFO_V1(pg_enc_int4_min);
Datum
    pg_enc_int4_min(PG_FUNCTION_ARGS){

    char* c1 = PG_GETARG_CSTRING(0);
    char* c2 = PG_GETARG_CSTRING(1);
    bool cmp = false;

    int32_t a, b;
    bytearray2int(c1, &a, INT32_LENGTH);
    bytearray2int(c2, &b, INT32_LENGTH);
    cmp = a > b;

    PG_RETURN_POINTER(c2);
}
PG_FUNCTION_INFO_V1(pg_enc_int4_max);
Datum
    pg_enc_int4_max(PG_FUNCTION_ARGS){

    char* c1 = PG_GETARG_CSTRING(0);
    char* c2 = PG_GETARG_CSTRING(1);
    bool cmp = false;

    int32_t a, b;
    bytearray2int(c1, &a, INT32_LENGTH);
    bytearray2int(c2, &b, INT32_LENGTH);
    cmp = a > b;

    PG_RETURN_POINTER(c1);
}


/*
 * The function computes the minimal element of array of enc_int4 elements
 * It is called by sql aggregate command MIN, which is firstly appends needed enc_int4 elements into array and then calls this function.
 * @input: an array of enc_int4 elements
 * @return: an enc_int4 result.
 */
PG_FUNCTION_INFO_V1(pg_enc_int4_minfinal);
Datum
    pg_enc_int4_minfinal(PG_FUNCTION_ARGS)
{
#if PRINT_ALL_FUNC
    static printed = false;
    if(!printed) {
        ereport(LOG, (errmsg("pg_enc_int4_minfinal here!")));
        printed = true;
    }
#endif
    ArrayType* v = PG_GETARG_ARRAYTYPE_P(0);
    int ans = 0;
    ArrayIterator array_iterator;
    ArrayMetaState* my_extra = (ArrayMetaState*)fcinfo->flinfo->fn_extra;
    bool isnull;
    Datum value;

    int ndims1 = ARR_NDIM(v); //array dimension
    int* dims1 = ARR_DIMS(v);
    int nitems = ArrayGetNItems(ndims1, dims1); //number of items in array

    char* pSrc1 = (char*)palloc((INT32_LENGTH) * sizeof(char));
    char* pTemp = (char*)palloc((INT32_LENGTH) * sizeof(char));
    char* pDst = (char*)palloc((INT32_LENGTH) * sizeof(char));

    array_iterator = array_create_iterator(v, 0, my_extra);
    array_iterate(array_iterator, &value, &isnull);

    // get first arg
    memcpy(pSrc1, DatumGetCString(value), INT32_LENGTH);

    int min_all = INT_MAX, a;
    bytearray2int(pSrc1, &a, INT32_LENGTH);
    if (a < min_all) min_all = a;

    while (array_iterate(array_iterator, &value, &isnull))
    {
        // get next arg
        memcpy(pTemp, DatumGetCString(value), INT32_LENGTH);
        bytearray2int(pTemp, &a, INT32_LENGTH);

        if (a < min_all)
            min_all = a;
    }

    int2bytearray((int32_t)min_all, pSrc1, INT32_LENGTH);

    pfree(pDst);
    pfree(pTemp);

    PG_RETURN_POINTER(pSrc1);
}

PG_FUNCTION_INFO_V1(pg_enc_int4_min_bulk);
Datum
    pg_enc_int4_min_bulk(PG_FUNCTION_ARGS)
{
#if PRINT_ALL_FUNC
    static printed = false;
    if(!printed) {
        ereport(LOG, (errmsg("pg_enc_int4_min_bulk here!")));
        printed = true;
    }
#endif
    // SAME AS pg_enc_int4_minfinal()
    ArrayType* v = PG_GETARG_ARRAYTYPE_P(0);
    int ans = 0;
    ArrayIterator array_iterator;
    ArrayMetaState* my_extra = (ArrayMetaState*)fcinfo->flinfo->fn_extra;
    bool isnull;
    Datum value;

    int ndims1 = ARR_NDIM(v); //array dimension
    int* dims1 = ARR_DIMS(v);
    int nitems = ArrayGetNItems(ndims1, dims1); //number of items in array

    char* pSrc1 = (char*)palloc((INT32_LENGTH) * sizeof(char));
    char* pTemp = (char*)palloc((INT32_LENGTH) * sizeof(char));
    char* pDst = (char*)palloc((INT32_LENGTH) * sizeof(char));

    array_iterator = array_create_iterator(v, 0, my_extra);
    array_iterate(array_iterator, &value, &isnull);

    // get first arg
    memcpy(pSrc1, DatumGetCString(value), INT32_LENGTH);

    int min_all = INT_MAX, a;
    bytearray2int(pSrc1, &a, INT32_LENGTH);
    if (a < min_all) min_all = a;

    while (array_iterate(array_iterator, &value, &isnull))
    {
        // get next arg
        memcpy(pTemp, DatumGetCString(value), INT32_LENGTH);
        bytearray2int(pTemp, &a, INT32_LENGTH);

        if (a < min_all)
            min_all = a;
    }

    int2bytearray((int32_t)min_all, pSrc1, INT32_LENGTH);

    pfree(pDst);
    pfree(pTemp);

    PG_RETURN_POINTER(pSrc1);
}
/*
 * The function computes the maximal element of array of enc_int4 elements
 * It is called by sql aggregate command MAX, which is firstly appends needed enc_int4 elements into array and then calls this function.
 * @input: array of enc_int4 elements
 * @return: an enc_int4 result.
 */
PG_FUNCTION_INFO_V1(pg_enc_int4_maxfinal);
Datum
    pg_enc_int4_maxfinal(PG_FUNCTION_ARGS)
{
#if PRINT_ALL_FUNC
    static printed = false;
    if(!printed) {
        ereport(LOG, (errmsg("pg_enc_int4_maxfinal here!")));
        printed = true;
    }
#endif
    ArrayType* v = PG_GETARG_ARRAYTYPE_P(0);
    int ans = 0;
    ArrayIterator array_iterator;
    ArrayMetaState* my_extra = (ArrayMetaState*)fcinfo->flinfo->fn_extra;
    bool isnull;
    Datum value;

    int ndims1 = ARR_NDIM(v); //array dimension
    int* dims1 = ARR_DIMS(v);
    int nitems = ArrayGetNItems(ndims1, dims1); //number of items in array

    char* pSrc1 = (char*)palloc((INT32_LENGTH) * sizeof(char));
    char* pTemp = (char*)palloc((INT32_LENGTH) * sizeof(char));
    char* pDst = (char*)palloc((INT32_LENGTH) * sizeof(char));

    array_iterator = array_create_iterator(v, 0, my_extra);
    array_iterate(array_iterator, &value, &isnull);

    // get first arg
    memcpy(pSrc1, DatumGetCString(value), INT32_LENGTH);
    int max_all = INT_MIN, a;
    bytearray2int(pSrc1, &a, INT32_LENGTH);
    if (a > max_all) max_all = a;

    while (array_iterate(array_iterator, &value, &isnull))
    {
        // get next arg
        memcpy(pTemp, DatumGetCString(value), INT32_LENGTH);
        bytearray2int(pTemp, &a, INT32_LENGTH);

        if (a > max_all)
            max_all = a;
    }
    int2bytearray((int32_t)max_all, pSrc1, INT32_LENGTH);

    pfree(pDst);
    pfree(pTemp);

    PG_RETURN_POINTER(pSrc1);
}

PG_FUNCTION_INFO_V1(pg_enc_int4_max_bulk);
Datum
    pg_enc_int4_max_bulk(PG_FUNCTION_ARGS)
{
#if PRINT_ALL_FUNC
    static printed = false;
    if(!printed) {
        ereport(LOG, (errmsg("pg_enc_int4_max_bulk here!")));
        printed = true;
    }
#endif
    // SAME AS pg_enc_int4_maxfinal()
    ArrayType* v = PG_GETARG_ARRAYTYPE_P(0);
    int ans = 0;
    ArrayIterator array_iterator;
    ArrayMetaState* my_extra = (ArrayMetaState*)fcinfo->flinfo->fn_extra;
    bool isnull;
    Datum value;

    int ndims1 = ARR_NDIM(v); //array dimension
    int* dims1 = ARR_DIMS(v);
    int nitems = ArrayGetNItems(ndims1, dims1); //number of items in array

    char* pSrc1 = (char*)palloc((INT32_LENGTH) * sizeof(char));
    char* pTemp = (char*)palloc((INT32_LENGTH) * sizeof(char));
    char* pDst = (char*)palloc((INT32_LENGTH) * sizeof(char));

    array_iterator = array_create_iterator(v, 0, my_extra);
    array_iterate(array_iterator, &value, &isnull);

    // get first arg
    memcpy(pSrc1, DatumGetCString(value), INT32_LENGTH);
    int max_all = INT_MIN, a;
    bytearray2int(pSrc1, &a, INT32_LENGTH);
    if (a > max_all) max_all = a;

    while (array_iterate(array_iterator, &value, &isnull))
    {
        // get next arg
        memcpy(pTemp, DatumGetCString(value), INT32_LENGTH);
        bytearray2int(pTemp, &a, INT32_LENGTH);

        if (a > max_all)
            max_all = a;
    }
    int2bytearray((int32_t)max_all, pSrc1, INT32_LENGTH);

    pfree(pDst);
    pfree(pTemp);

    PG_RETURN_POINTER(pSrc1);
}

/*
 * The function converts an integer to enc_int4 value. This function is calles by sql function CAST.
 * @input: int4
 * @return: an enc_int4 result.
 */
PG_FUNCTION_INFO_V1(pg_int4_to_enc_int4);
Datum
    pg_int4_to_enc_int4(PG_FUNCTION_ARGS)
{
#if PRINT_ALL_FUNC
    static printed7 = false;
    if(!printed7) {
        ereport(LOG, (errmsg("pg_int4_to_enc_int4 here!")));
        printed7 = true;
    }
#endif
    int c1 = PG_GETARG_INT32(0);
    char* pDst = (char*)palloc((INT32_LENGTH) * sizeof(char));
    int2bytearray((int32_t)c1, pDst, INT32_LENGTH);
    PG_RETURN_POINTER((const char*)pDst);
}

/*
 * The function converts an integer(8 bytes, known as bigint) to enc_int4 value. This function is calles by sql function CAST.
 * @input: int8
 * @return: an enc_int4 result.
 */
PG_FUNCTION_INFO_V1(pg_int8_to_enc_int4);
Datum
    pg_int8_to_enc_int4(PG_FUNCTION_ARGS)
{
#if PRINT_ALL_FUNC
    static printed8 = false;
    if(!printed8) {
        ereport(LOG, (errmsg("pg_int8_to_enc_int4 here!")));
        printed8 = true;
    }
#endif
    int64 c1 = PG_GETARG_INT64(0);
    char* pDst = (char*)palloc((INT32_LENGTH) * sizeof(char));
    if (c1 < INT_MIN || c1 > INT_MAX)
            ereport(ERROR, (errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE), errmsg("value \"%li\" is out of range for type %s", c1, "integer")));
        
    int2bytearray((int32_t)c1, pDst, INT32_LENGTH);
    PG_RETURN_POINTER((const char*)pDst);
}
