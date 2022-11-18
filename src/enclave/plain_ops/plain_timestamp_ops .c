#include <plain_timestamp_ops.h>
#include <defs.h>

int plain_timestamp_cmp(TIMESTAMP left, TIMESTAMP right){
    return (left == right) ? 0 : (left < right) ? -1 : 1;
}