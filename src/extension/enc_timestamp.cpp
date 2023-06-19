#include "extension.hpp"


extern "C" {
PG_FUNCTION_INFO_V1(pg_enc_timestamp_in);
PG_FUNCTION_INFO_V1(pg_enc_timestamp_out);
PG_FUNCTION_INFO_V1(pg_enc_timestamp_eq);
PG_FUNCTION_INFO_V1(pg_enc_timestamp_ne);
PG_FUNCTION_INFO_V1(pg_enc_timestamp_lt);
PG_FUNCTION_INFO_V1(pg_enc_timestamp_le);
PG_FUNCTION_INFO_V1(pg_enc_timestamp_gt);
PG_FUNCTION_INFO_V1(pg_enc_timestamp_ge);
PG_FUNCTION_INFO_V1(pg_enc_timestamp_cmp);
PG_FUNCTION_INFO_V1(date_part);

PG_FUNCTION_INFO_V1(pg_enc_timestamp_encrypt);
PG_FUNCTION_INFO_V1(pg_enc_timestamp_decrypt);
}


static TimeOffset time2t(const int hour, const int min, const int sec, const fsec_t fsec)
{
    return (((hour * MINS_PER_HOUR) + min) * SECS_PER_MINUTE) + sec + fsec;
}

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

    default:
        elog(ERROR, "unexpected dtype %d while parsing timestamp \"%s\"",
             dtype, str);
        TIMESTAMP_NOEND(result);
    }

    return result;
}



Datum
    pg_enc_timestamp_in(PG_FUNCTION_ARGS)
{
    char* pSrc = PG_GETARG_CSTRING(0);
    uint64_t index = 0;
    if(pSrc[0] == FLAG_CHAR){
        index = strtoull(pSrc + 1, NULL, 10);
    }else {
        TIMESTAMP dst= pg_timestamp_in(pSrc);
        index = makeIndex(0, insertTs(dst));
    }
    
    PG_RETURN_DATUM(index);
}

Datum
    pg_enc_timestamp_out(PG_FUNCTION_ARGS)
{
    uint64_t index = PG_GETARG_DATUM(0);
    char* result = (char*)palloc(MAXDATELEN + 1);
    TIMESTAMP timestamp = getTs(index);
    struct pg_tm tt, *tm = &tt;
    fsec_t fsec;
    if (timestamp2tm(timestamp, NULL, tm, &fsec, NULL, NULL) == 0)
        EncodeDateTime(tm, fsec, false, 0, NULL, 1, result);
    else
    {
        ereport(ERROR,
                (errcode(ERRCODE_DATETIME_VALUE_OUT_OF_RANGE),
                    errmsg("timestamp out of range")));
    }
    PG_RETURN_CSTRING(result);
}

Datum
    pg_enc_timestamp_encrypt(PG_FUNCTION_ARGS)
{
    PG_RETURN_DATUM(0);
}

Datum
    pg_enc_timestamp_decrypt(PG_FUNCTION_ARGS)
{
    PG_RETURN_DATUM(0);
}

Datum
    pg_enc_timestamp_eq(PG_FUNCTION_ARGS)
{
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    int cmp = enc_timestamp_cmp(index1, index2);
    PG_RETURN_BOOL(cmp == 0);
}

Datum
    pg_enc_timestamp_ne(PG_FUNCTION_ARGS)
{
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    int cmp = enc_timestamp_cmp(index1, index2);
    PG_RETURN_BOOL(cmp != 0);
}

Datum
    pg_enc_timestamp_lt(PG_FUNCTION_ARGS)
{
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    int cmp = enc_timestamp_cmp(index1, index2);
    PG_RETURN_BOOL(cmp < 0);
}

Datum
    pg_enc_timestamp_le(PG_FUNCTION_ARGS)
{
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    int cmp = enc_timestamp_cmp(index1, index2);
    PG_RETURN_BOOL(cmp <= 0);
}

Datum
    pg_enc_timestamp_gt(PG_FUNCTION_ARGS)
{
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    int cmp = enc_timestamp_cmp(index1, index2);
    PG_RETURN_BOOL(cmp > 0);
}

Datum
    pg_enc_timestamp_ge(PG_FUNCTION_ARGS)
{
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    int cmp = enc_timestamp_cmp(index1, index2);
    PG_RETURN_BOOL(cmp >= 0);
}

Datum
    pg_enc_timestamp_cmp(PG_FUNCTION_ARGS)
{
    uint64_t index1 = PG_GETARG_DATUM(0);
    uint64_t index2 = PG_GETARG_DATUM(1);
    int cmp = enc_timestamp_cmp(index1, index2);
    PG_RETURN_BOOL(cmp);
}


Datum
    date_part(PG_FUNCTION_ARGS)
{
    char* get = text_to_cstring(PG_GETARG_TEXT_P(0));
    uint64_t index = PG_GETARG_DATUM(1);
    if (strcmp(get, "year") != 0) {
        ereport(ERROR, (errmsg("Only date_part('year', enc_timestamp) is currently implemented.")));
    }
    uint64_t res = enc_timestamp_extract_year(index);
    PG_RETURN_DATUM(res);

}