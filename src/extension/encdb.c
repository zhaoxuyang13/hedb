#include <stdafx.h>
#include <extension.h>

PG_MODULE_MAGIC;


bool debugMode = true;


PG_FUNCTION_INFO_V1(launch);
Datum
    launch(PG_FUNCTION_ARGS)
{
    ereport(LOG, (errmsg("launch was called")));

    int64_t ret = 0;
    
    PG_RETURN_INT32(ret);
}

PG_FUNCTION_INFO_V1(enable_debug_mode);
Datum
    enable_debug_mode(PG_FUNCTION_ARGS)
{
    debugMode = true;
    PG_RETURN_INT32(0);
}