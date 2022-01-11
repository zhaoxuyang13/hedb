/*
 * a simple UDF for timestamp
 */
#include "extension.hpp"

#ifdef __cplusplus
extern "C" {
#endif
PG_FUNCTION_INFO_V1(pg_enc_timestamp_in);
PG_FUNCTION_INFO_V1(pg_enc_timestamp_out);
PG_FUNCTION_INFO_V1(pg_enc_timestamp_encrypt);
PG_FUNCTION_INFO_V1(pg_enc_timestamp_decrypt);
PG_FUNCTION_INFO_V1(pg_enc_timestamp_eq);
PG_FUNCTION_INFO_V1(pg_enc_timestamp_ne);
PG_FUNCTION_INFO_V1(pg_enc_timestamp_lt);
PG_FUNCTION_INFO_V1(pg_enc_timestamp_le);
PG_FUNCTION_INFO_V1(pg_enc_timestamp_gt);
PG_FUNCTION_INFO_V1(pg_enc_timestamp_ge);
PG_FUNCTION_INFO_V1(pg_enc_timestamp_cmp);
#ifdef __cplusplus
}
#endif

static TimeOffset time2t(const int hour, const int min, const int sec, const fsec_t fsec)
{
    return (((hour * MINS_PER_HOUR) + min) * SECS_PER_MINUTE) + sec + fsec;
}

/* Convert a string to internal timestamp type. This function based on native postgres function 'timestamp_in'
 * @input: string as a postgres argument
`* @return: timestamp
*/
Timestamp pg_timestamp_in(char* str)
{

    Timestamp result;
    char workbuf[MAXDATELEN + MAXDATEFIELDS];
    char* field[MAXDATEFIELDS];
    int ftype[MAXDATEFIELDS];
    int dterr;
    int nf;
    int tz;
    int dtype;
    fsec_t fsec;
    struct pg_tm tt, *tm = &tt;
    char buf[MAXDATELEN + 1];
    char src_byte[TIMESTAMP_LENGTH];
    int resp;

    dterr = ParseDateTime(str, workbuf, sizeof(workbuf), field, ftype, MAXDATEFIELDS, &nf);

    if (dterr == 0)
        dterr = DecodeDateTime(field, ftype, nf, &dtype, tm, &fsec, &tz);
    if (dterr != 0)
        DateTimeParseError(dterr, str, "timestamp");

    switch (dtype)
    {
    case DTK_DATE:
        if (tm2timestamp(tm, fsec, NULL, &result) != 0)
            ereport(ERROR,
                    (errcode(ERRCODE_DATETIME_VALUE_OUT_OF_RANGE),
                     errmsg("timestamp out of range: \"%s\"", str)));
        break;

    case DTK_EPOCH:
        result = SetEpochTimestamp();
        break;

    case DTK_LATE:
        TIMESTAMP_NOEND(result);
        break;

    case DTK_EARLY:
        TIMESTAMP_NOBEGIN(result);
        break;

//    case DTK_INVALID:
//        ereport(ERROR,
//                (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
//                 errmsg("date/time value \"%s\" is no longer supported", str)));

//        TIMESTAMP_NOEND(result);
//        break;

    default:
        elog(ERROR, "unexpected dtype %d while parsing timestamp \"%s\"",
             dtype, str);
        TIMESTAMP_NOEND(result);
    }

    return result;
}

/*
 * The function converts string to enc_timestamp. It is called by dbms every time it parses a query and finds an enc_timestamp element.
 * @input: string as a postgres arg
 * @return: enc_timestamp element as a string
 */
Datum
    pg_enc_timestamp_in(PG_FUNCTION_ARGS)
{
    // ereport(INFO, (errmsg("pg_enc_timestamp_in here!")));
    char* pSrc = PG_GETARG_CSTRING(0);
    TIMESTAMP dst;
    char* src = (char*)palloc(TIMESTAMP_LENGTH * sizeof(char));
    dst = pg_timestamp_in(pSrc);
    memcpy(src, &dst, TIMESTAMP_LENGTH * sizeof(char));
    PG_RETURN_CSTRING(src);
}
/*
 * The function converts enc_timestamp element to a string. If flag debugDecryption is true it decrypts the string and return unencrypted result.
 * @input: enc_timestamp element
 * @return: string
 */
Datum
    pg_enc_timestamp_out(PG_FUNCTION_ARGS)
{
    // ereport(INFO, (errmsg("pg_enc_timestamp_out here!")));
    char* c1 = PG_GETARG_CSTRING(0);
    TIMESTAMP timestamp;
    char* result = (char*)palloc(TIMESTAMP_LENGTH * sizeof(char));
    struct pg_tm tt, *tm = &tt;
    fsec_t fsec;
    char buf[MAXDATELEN + 1];
    memcpy(&timestamp, c1, TIMESTAMP_LENGTH * sizeof(char));
    if (timestamp2tm(timestamp, NULL, tm, &fsec, NULL, NULL) == 0)
        EncodeDateTime(tm, fsec, false, 0, NULL, 1, buf);
    else
    {
        ereport(ERROR,
                (errcode(ERRCODE_DATETIME_VALUE_OUT_OF_RANGE),
                    errmsg("timestamp out of range")));
    }
    result = pstrdup(buf);
    PG_RETURN_CSTRING(result);
}

/*
 *  Gets a string as a timestamp element, encrypts it and return enc_timestamp element as a string.
 *   Converts the input string to a int64 element, encrypts one and return base64 encrypted result.
 *    @input: string
 *    @return: a string describing enc_timestamp element.
 */
Datum
    pg_enc_timestamp_encrypt(PG_FUNCTION_ARGS)
{
    char* arg = PG_GETARG_CSTRING(0);
#ifdef NOT_USED
    Oid typelem = PG_GETARG_OID(1);
#endif
    int32 typmod = PG_GETARG_INT32(2);

    Timestamp result;
    char* src = (char*)palloc(TIMESTAMP_LENGTH);
    result = pg_timestamp_in(arg);
    memcpy(src, &result, sizeof(TIMESTAMP_LENGTH));
    PG_RETURN_CSTRING(src);
}

/*
 *  Gets a string as a enc_timestamp element, decrypts it and return timestamp element as a string.
 *  @input: enc_timestamp element
 *   @return: string
 */
Datum
    pg_enc_timestamp_decrypt(PG_FUNCTION_ARGS)
{
    char* c1 = PG_GETARG_CSTRING(0);
    Timestamp timestamp;
    int resp;
    char* result;
    struct pg_tm tt,
        *tm = &tt;
    fsec_t fsec;
    char buf[MAXDATELEN + 1];
    memcpy(&timestamp, c1, TIMESTAMP_LENGTH);
    if (timestamp2tm(timestamp, NULL, tm, &fsec, NULL, NULL) == 0)
        EncodeDateTime(tm, fsec, false, 0, NULL, 1, buf);
    else
    {
        ereport(ERROR,
                (errcode(ERRCODE_DATETIME_VALUE_OUT_OF_RANGE),
                 errmsg("timestamp out of range")));
    }
    result = pstrdup(buf);
    PG_RETURN_CSTRING(result);
}

/*
 * The function checks if the first input enc_timestamp is equal to the second one.
 * It is called by binary operator '=' defined in sql extension.
 * @input: two enc_timestamp values
 * @return: true, if the first integer is equal to the second one.
 *       false, otherwise
 */
Datum
    pg_enc_timestamp_eq(PG_FUNCTION_ARGS)
{
    char* c1 = PG_GETARG_CSTRING(0);
    char* c2 = PG_GETARG_CSTRING(1);
    TIMESTAMP timestp1;
    TIMESTAMP timestp2;
    memcpy(&timestp1, c1, TIMESTAMP_LENGTH);
    memcpy(&timestp2, c2, TIMESTAMP_LENGTH);

    int ans = (timestp1 == timestp2) ? 0 : ((timestp1 < timestp2) ? -1 : 1);
    PG_RETURN_BOOL((ans == 0) ? true : false);
}

/*
 * The function checks if the first input enc_timestamp is not equal to the second one.
 * It is called by binary operator '=' defined in sql extension.
 * @input: two enc_timestamp values
 * @return: true, if the first integer is equal to the second one.
 *       false, otherwise
 */
Datum
    pg_enc_timestamp_ne(PG_FUNCTION_ARGS)
{
    char* c1 = PG_GETARG_CSTRING(0);
    char* c2 = PG_GETARG_CSTRING(1);
    TIMESTAMP timestp1;
    TIMESTAMP timestp2;
    memcpy(&timestp1, c1, TIMESTAMP_LENGTH);
    memcpy(&timestp2, c2, TIMESTAMP_LENGTH);

    int ans = (timestp1 == timestp2) ? 0 : ((timestp1 < timestp2) ? -1 : 1);
    PG_RETURN_BOOL((ans != 0) ? true : false);
}

/*
 * The function checks if the first input enc_timestamp is less to the second one.
 * It is called by binary operator '=' defined in sql extension.
 * @input: two enc_timestamp values
 * @return: true, if the first integer is equal to the second one.
 *       false, otherwise
 */
Datum
    pg_enc_timestamp_lt(PG_FUNCTION_ARGS)
{
    char* c1 = PG_GETARG_CSTRING(0);
    char* c2 = PG_GETARG_CSTRING(1);
    TIMESTAMP timestp1;
    TIMESTAMP timestp2;
    memcpy(&timestp1, c1, TIMESTAMP_LENGTH);
    memcpy(&timestp2, c2, TIMESTAMP_LENGTH);

    int ans = (timestp1 == timestp2) ? 0 : ((timestp1 < timestp2) ? -1 : 1);
    PG_RETURN_BOOL((ans == -1) ? true : false);
}

/*
 * The function checks if the first input enc_timestamp is less or equal to the second one.
 * It is called by binary operator '=' defined in sql extension.
 * @input: two enc_timestamp values
 * @return: true, if the first integer is equal to the second one.
 *       false, otherwise
 */
Datum
    pg_enc_timestamp_le(PG_FUNCTION_ARGS)
{
    char* c1 = PG_GETARG_CSTRING(0);
    char* c2 = PG_GETARG_CSTRING(1);
    TIMESTAMP timestp1;
    TIMESTAMP timestp2;
    memcpy(&timestp1, c1, TIMESTAMP_LENGTH);
    memcpy(&timestp2, c2, TIMESTAMP_LENGTH);

    int ans = (timestp1 == timestp2) ? 0 : ((timestp1 < timestp2) ? -1 : 1);
    PG_RETURN_BOOL((ans <= 0) ? true : false);
}

/*
 * The function checks if the first input enc_timestamp is greater to the second one.
 * It is called by binary operator '=' defined in sql extension.
 * @input: two enc_timestamp values
 * @return: true, if the first integer is equal to the second one.
 *       false, otherwise
 */
Datum
    pg_enc_timestamp_gt(PG_FUNCTION_ARGS)
{
    char* c1 = PG_GETARG_CSTRING(0);
    char* c2 = PG_GETARG_CSTRING(1);
    TIMESTAMP timestp1;
    TIMESTAMP timestp2;
    memcpy(&timestp1, c1, TIMESTAMP_LENGTH);
    memcpy(&timestp2, c2, TIMESTAMP_LENGTH);

    int ans = (timestp1 == timestp2) ? 0 : ((timestp1 < timestp2) ? -1 : 1);
    PG_RETURN_BOOL((ans > 0) ? true : false);
}

/*
 * The function checks if the first input enc_timestamp is greater or equal to the second one.
 * It is called by binary operator '=' defined in sql extension.
 * @input: two enc_timestamp values
 * @return: true, if the first integer is equal to the second one.
 *       false, otherwise
 */
Datum
    pg_enc_timestamp_ge(PG_FUNCTION_ARGS)
{
    char* c1 = PG_GETARG_CSTRING(0);
    char* c2 = PG_GETARG_CSTRING(1);
    TIMESTAMP timestp1;
    TIMESTAMP timestp2;
    memcpy(&timestp1, c1, TIMESTAMP_LENGTH);
    memcpy(&timestp2, c2, TIMESTAMP_LENGTH);

    int ans = (timestp1 == timestp2) ? 0 : ((timestp1 < timestp2) ? -1 : 1);
    PG_RETURN_BOOL((ans >= 0) ? true : false);
}

/*
 * The function compares two values. It is called mostly during index building.
 * It compares them as a timestamp (int64 elements) and return {-1,0,1} as a result.
 * @input: two enc_timestamp values
 * @return: -1, 0 ,1
 */
Datum
    pg_enc_timestamp_cmp(PG_FUNCTION_ARGS)
{
    char* c1 = PG_GETARG_CSTRING(0);
    char* c2 = PG_GETARG_CSTRING(1);

    TIMESTAMP timestp1;
    TIMESTAMP timestp2;
    memcpy(&timestp1, c1, TIMESTAMP_LENGTH);
    memcpy(&timestp2, c2, TIMESTAMP_LENGTH);

    int ans = (timestp1 == timestp2) ? 0 : ((timestp1 < timestp2) ? -1 : 1);

    PG_RETURN_INT32(ans);
}
