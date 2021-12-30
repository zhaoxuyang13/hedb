#include <stdafx.h>
#include <extension.h>

PG_MODULE_MAGIC;

void print_info(const char * str){
    ereport(INFO,(errmsg("%s", str)));
}

void print_error(const char * str){
    ereport(ERROR,(errmsg("%s", str)));
}
bool debugMode = true;


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