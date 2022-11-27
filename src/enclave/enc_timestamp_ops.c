#include "enc_timestamp_ops.h"
#include <limits.h>

int enc_timestamp_cmp(EncTimestampCmpRequestData *req)
{
    int resp = 0;

    TIMESTAMP left, right;

    resp = decrypt_bytes((uint8_t *) &req->left, sizeof(req->left),(uint8_t*) &left, sizeof(left));
    if (resp != 0)
        return resp;

    resp = decrypt_bytes((uint8_t *) &req->right, sizeof(req->right),(uint8_t*) &right, sizeof(right));
    if (resp != 0)
        return resp;

    req->cmp = (left == right) ? 0 : (left < right) ? -1 : 1;

    // printf("%d, %d, %d, %d\n",req->common.reqType, left,right,req->cmp);
    return resp;
}


#define TMODULO(t,q,u) \
do { \
    (q) = ((t) / (u)); \
    if ((q) != 0) (t) -= ((q) * (u)); \
} while(0)

#define INT64CONST(x) (x##L)
#define USECS_PER_DAY INT64CONST(86400000000)
#define POSTGRES_EPOCH_JDATE 2451545

int year_from_timestamp(int64_t timestamp)
{

    int64_t date;
    unsigned int quad;
    unsigned int extra;
    int year;

    TMODULO(timestamp, date, USECS_PER_DAY);
    if (timestamp < INT64CONST(0))
    {
        timestamp += USECS_PER_DAY;
        date -= 1;
    }

    /* add offset to go from J2000 back to standard Julian date */
    date += POSTGRES_EPOCH_JDATE;

    /* Julian day routine does not work for negative Julian days */
    if (date < 0 || date > (int64_t)INT_MAX)
        return -1;

    date += 32044;
    quad = date / 146097;
    extra = (date - quad * 146097) * 4 + 3;

    date += 60 + quad * 3 + extra / 146097;
    quad = date / 1461;
    date -= quad * 1461;

    year = date * 4 / 1461;
    year += quad * 4;
    return year - 4800;
}


int enc_timestamp_extract_year(EncTimestampExtractYearRequestData *req)
{
    int resp = 0;
    TIMESTAMP t;
    int year;
    resp = decrypt_bytes((uint8_t *) &req->in, sizeof(req->in),(uint8_t*) &t, sizeof(t));
    if (resp != 0)
        return resp;
    year = year_from_timestamp(t);
    
    resp = encrypt_bytes((uint8_t *) &year, sizeof(year), (uint8_t *) &req->res, sizeof(req->res));
    return resp;
}
