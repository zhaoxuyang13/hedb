#include "enc_ops.hpp"
#include "extension.hpp"


int enc_timestamp_cmp(uint64_t left, uint64_t right){
    TIMESTAMP l = getTs(left);
    TIMESTAMP r = getTs(right);
    // ereport(INFO, (errmsg("cmp %ld %ld", l, r)));
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

    date += POSTGRES_EPOCH_JDATE;

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
    TIMESTAMP t = getTs(timestamp);
    int year = timestamp_extract_year(t);
    uint32_t key = insertInt(year);
    return makeIndex(0, key);
}


