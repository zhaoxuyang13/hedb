#include "enc_timestamp_ops.hpp"

int enc_timestamp_cmp(uint64_t left, uint64_t right){
    int l = getTs(left);
    int r = getTs(right);
    return l > r ? 1 : (l < r ? -1 : 0);
}

static int timestamp_extract_year(int64_t timestamp)
{

    int64_t date;
    unsigned int quad;
    unsigned int extra;
    int year;

    TMODULO(timestamp, date, USECS_PER_DAY);
    if (timestamp < INT64CONST(0)) {
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


uint64_t enc_timestamp_extract_year(uint64_t timestamp){
    int t = getTs(timestamp);
    int year = timestamp_extract_year(t);
    uint32_t key = insertInt(year);
    return makeIndex(0, key);
}

