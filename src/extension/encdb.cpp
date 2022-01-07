#include <stdafx.h>
#include <extension.h>
#include <extension_helper.hpp>
#include <stdarg.h>
extern "C"{
PG_MODULE_MAGIC;
PG_FUNCTION_INFO_V1(launch);
PG_FUNCTION_INFO_V1(enable_debug_mode);
}

bool debugMode = true;

void print_info(const char *str,...)
{
    ereport(INFO, (errmsg(str)));
}
void print_error(const char *fmt,...)
{
    ereport(ERROR, (errmsg(fmt)));
}
Datum
    launch(PG_FUNCTION_ARGS)
{
    ereport(LOG, (errmsg("launch was called")));

    int64_t ret = 0;
    
    PG_RETURN_INT32(ret);
}

Datum
    enable_debug_mode(PG_FUNCTION_ARGS)
{
    debugMode = true;
    PG_RETURN_INT32(0);
}