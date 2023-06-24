/*
 * a simple UDF for int4
 */
#include <extension.hpp>
#include "kv.hpp"

#define PRINT_ALL_FUNC 0

extern "C" {
PG_FUNCTION_INFO_V1(pg_enc_int4_in);
PG_FUNCTION_INFO_V1(pg_enc_int4_out);
PG_FUNCTION_INFO_V1(pg_enc_int4_add);
PG_FUNCTION_INFO_V1(pg_enc_int4_sub);
PG_FUNCTION_INFO_V1(pg_enc_int4_mult);
PG_FUNCTION_INFO_V1(pg_enc_int4_div);
PG_FUNCTION_INFO_V1(pg_enc_int4_pow);
PG_FUNCTION_INFO_V1(pg_enc_int4_mod);
PG_FUNCTION_INFO_V1(pg_enc_int4_eq);
PG_FUNCTION_INFO_V1(pg_enc_int4_ne);
PG_FUNCTION_INFO_V1(pg_enc_int4_le);
PG_FUNCTION_INFO_V1(pg_enc_int4_lt);
PG_FUNCTION_INFO_V1(pg_enc_int4_ge);
PG_FUNCTION_INFO_V1(pg_enc_int4_gt);
PG_FUNCTION_INFO_V1(pg_enc_int4_cmp);
PG_FUNCTION_INFO_V1(pg_enc_int4_sum_bulk);
PG_FUNCTION_INFO_V1(pg_enc_int4_avg_bulk);
PG_FUNCTION_INFO_V1(pg_enc_int4_min);
PG_FUNCTION_INFO_V1(pg_enc_int4_max);
// PG_FUNCTION_INFO_V1(pg_enc_int4_min_bulk);
// PG_FUNCTION_INFO_V1(pg_enc_int4_max_bulk);

PG_FUNCTION_INFO_V1(pg_int4_to_enc_int4);
PG_FUNCTION_INFO_V1(pg_int8_to_enc_int4);
PG_FUNCTION_INFO_V1(pg_enc_int4_encrypt);
PG_FUNCTION_INFO_V1(pg_enc_int4_decrypt);
}

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
    char* pSrc = PG_GETARG_CSTRING(0);
    uint64_t index = 0;
    if(pSrc[0] == FLAG_CHAR){ // if the number begin with 0, its a kv.
        index = strtoull(pSrc + 1, NULL, 10);
    }else {
        int src = pg_atoi(pSrc, sizeof(int), '\0');
        index = makeIndex(0, insertInt(src));
    }
    PG_RETURN_DATUM(index);
}

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
    uint64_t index = PG_GETARG_DATUM(0);
    char* str = (char*)palloc(32 * sizeof(char));

    int ans = getInt(index);
    sprintf(str, "%d", ans);
    PG_RETURN_POINTER(str);
}


Datum
    pg_int4_to_enc_int4(PG_FUNCTION_ARGS)
{
    int in = PG_GETARG_INT32(0);
    uint64_t index = makeIndex(0, insertInt(in));
    PG_RETURN_DATUM(index);
}

Datum
    pg_int8_to_enc_int4(PG_FUNCTION_ARGS)
{
    int64 in = PG_GETARG_INT64(0);
    uint64_t index = makeIndex(0, insertInt(in));
    PG_RETURN_DATUM(index);
}





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
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    
    uint64_t index = enc_int_add(index1, index2);

    PG_RETURN_DATUM(index);
}

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
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    
    uint64_t index = enc_int_sub(index1, index2);

    PG_RETURN_DATUM(index);
}

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
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    
    uint64_t index = enc_int_mult(index1, index2);

    PG_RETURN_DATUM(index);
}

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
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    
    uint64_t index = enc_int_div(index1, index2);

    PG_RETURN_DATUM(index);
}

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
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    
    uint64_t index = enc_int_pow(index1, index2);

    PG_RETURN_DATUM(index);
}

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
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    
    uint64_t index = enc_int_mod(index1, index2);

    PG_RETURN_DATUM(index);
}

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
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    
    int32_t result = enc_int_cmp(index1, index2);

    PG_RETURN_INT32(result);
}

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
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    
    int32_t result = enc_int_cmp(index1, index2);
    PG_RETURN_BOOL(result == 0);
}

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
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    
    int32_t result = enc_int_cmp(index1, index2);
    PG_RETURN_BOOL(result != 0);
}

Datum
    pg_enc_int4_lt(PG_FUNCTION_ARGS)
{
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    
    int32_t result = enc_int_cmp(index1, index2);
    PG_RETURN_BOOL(result < 0);
}

Datum
    pg_enc_int4_le(PG_FUNCTION_ARGS)
{
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    
    int32_t result = enc_int_cmp(index1, index2);
    PG_RETURN_BOOL(result <= 0);
}

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
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    
    int32_t result = enc_int_cmp(index1, index2);
    PG_RETURN_BOOL(result > 0);
}

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
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    
    int32_t result = enc_int_cmp(index1, index2);
    PG_RETURN_BOOL(result >= 0);
}


Datum
    pg_enc_int4_encrypt(PG_FUNCTION_ARGS)
{
    PG_RETURN_DATUM(0);
}

Datum
    pg_enc_int4_decrypt(PG_FUNCTION_ARGS)
{
    PG_RETURN_DATUM(0);
}


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
    ArrayType* v = PG_GETARG_ARRAYTYPE_P(0);
    ArrayIterator array_iterator;
    ArrayMetaState* my_extra = (ArrayMetaState*)fcinfo->flinfo->fn_extra;
    bool isnull;
    Datum value;

    array_iterator = array_create_iterator(v, 0, my_extra);
    array_iterate(array_iterator, &value, &isnull);

    uint64_t bulk_array[BULK_SIZE];
    int count = 1;
    bulk_array[0] = DatumGetUInt64(value);
    while (array_iterate(array_iterator, &value, &isnull))
    {
        if(count == BULK_SIZE) {
            uint64_t tmp = enc_int_sum_bulk(BULK_SIZE, bulk_array);
            bulk_array[0] = tmp;
            count = 1;
        }
        bulk_array[count] = DatumGetUInt64(value);
        count ++;
    }
    uint64_t res = enc_int_sum_bulk(count, bulk_array);
    PG_RETURN_DATUM(res);
}

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

    array_iterator = array_create_iterator(v, 0, my_extra);
    array_iterate(array_iterator, &value, &isnull);

    uint64_t bulk_array[BULK_SIZE];
    int count = 1;
    bulk_array[0] = DatumGetUInt64(value);
    while (array_iterate(array_iterator, &value, &isnull))
    {
        if(count == BULK_SIZE) {
            uint64_t tmp = enc_int_sum_bulk(BULK_SIZE, bulk_array);
            bulk_array[0] = tmp;
            count = 1;
        }
        bulk_array[count] = DatumGetUInt64(value);
        count ++;
    }
    
    uint64_t sum = enc_int_sum_bulk(count, bulk_array);
    uint64_t div = makeIndex(0, insertInt(nitems)); // TODO
    uint64_t res = enc_int_div(sum, div);

    PG_RETURN_DATUM(res);
}


Datum pg_enc_int4_min(PG_FUNCTION_ARGS){
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    
    int32_t result = enc_int_cmp(index1, index2);
    if (result < 0) {
        PG_RETURN_DATUM(index1);
    } else {
        PG_RETURN_DATUM(index2);
    }
}

Datum pg_enc_int4_max(PG_FUNCTION_ARGS){
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);

    int32_t result = enc_int_cmp(index1, index2);
    if (result > 0) {
        PG_RETURN_DATUM(index1);
    } else {
        PG_RETURN_DATUM(index2);
    }
}


// Datum
//     pg_enc_int4_min_bulk(PG_FUNCTION_ARGS)
// {
// #if PRINT_ALL_FUNC
//     static printed = false;
//     if(!printed) {
//         ereport(LOG, (errmsg("pg_enc_int4_min_bulk here!")));
//         printed = true;
//     }
// #endif
//     // SAME AS pg_enc_int4_minfinal()
//     ArrayType* v = PG_GETARG_ARRAYTYPE_P(0);
//     int ans = 0;
//     ArrayIterator array_iterator;
//     ArrayMetaState* my_extra = (ArrayMetaState*)fcinfo->flinfo->fn_extra;
//     bool isnull;
//     Datum value;

//     int ndims1 = ARR_NDIM(v); //array dimension
//     int* dims1 = ARR_DIMS(v);
//     int nitems = ArrayGetNItems(ndims1, dims1); //number of items in array

//     char* pSrc1 = (char*)palloc((INT32_LENGTH) * sizeof(char));
//     char* pTemp = (char*)palloc((INT32_LENGTH) * sizeof(char));
//     char* pDst = (char*)palloc((INT32_LENGTH) * sizeof(char));

//     array_iterator = array_create_iterator(v, 0, my_extra);
//     array_iterate(array_iterator, &value, &isnull);

//     // get first arg
//     memcpy(pSrc1, DatumGetCString(value), INT32_LENGTH);

//     int min_all = INT_MAX, a;
//     bytearray2int(pSrc1, &a, INT32_LENGTH);
//     if (a < min_all) min_all = a;

//     while (array_iterate(array_iterator, &value, &isnull))
//     {
//         // get next arg
//         memcpy(pTemp, DatumGetCString(value), INT32_LENGTH);
//         bytearray2int(pTemp, &a, INT32_LENGTH);

//         if (a < min_all)
//             min_all = a;
//     }

//     int2bytearray((int32_t)min_all, pSrc1, INT32_LENGTH);

//     pfree(pDst);
//     pfree(pTemp);

//     PG_RETURN_POINTER(pSrc1);
// }

// Datum
//     pg_enc_int4_max_bulk(PG_FUNCTION_ARGS)
// {
// #if PRINT_ALL_FUNC
//     static printed = false;
//     if(!printed) {
//         ereport(LOG, (errmsg("pg_enc_int4_max_bulk here!")));
//         printed = true;
//     }
// #endif
//     // SAME AS pg_enc_int4_maxfinal()
//     ArrayType* v = PG_GETARG_ARRAYTYPE_P(0);
//     int ans = 0;
//     ArrayIterator array_iterator;
//     ArrayMetaState* my_extra = (ArrayMetaState*)fcinfo->flinfo->fn_extra;
//     bool isnull;
//     Datum value;

//     int ndims1 = ARR_NDIM(v); //array dimension
//     int* dims1 = ARR_DIMS(v);
//     int nitems = ArrayGetNItems(ndims1, dims1); //number of items in array

//     char* pSrc1 = (char*)palloc((INT32_LENGTH) * sizeof(char));
//     char* pTemp = (char*)palloc((INT32_LENGTH) * sizeof(char));
//     char* pDst = (char*)palloc((INT32_LENGTH) * sizeof(char));

//     array_iterator = array_create_iterator(v, 0, my_extra);
//     array_iterate(array_iterator, &value, &isnull);

//     // get first arg
//     memcpy(pSrc1, DatumGetCString(value), INT32_LENGTH);
//     int max_all = INT_MIN, a;
//     bytearray2int(pSrc1, &a, INT32_LENGTH);
//     if (a > max_all) max_all = a;

//     while (array_iterate(array_iterator, &value, &isnull))
//     {
//         // get next arg
//         memcpy(pTemp, DatumGetCString(value), INT32_LENGTH);
//         bytearray2int(pTemp, &a, INT32_LENGTH);

//         if (a > max_all)
//             max_all = a;
//     }
//     int2bytearray((int32_t)max_all, pSrc1, INT32_LENGTH);

//     pfree(pDst);
//     pfree(pTemp);

//     PG_RETURN_POINTER(pSrc1);
// }

