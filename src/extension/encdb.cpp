#include <stdafx.h>
#include <extension.h>
#include <extension_helper.h>
extern "C"{
PG_MODULE_MAGIC;
PG_FUNCTION_INFO_V1(launch);
PG_FUNCTION_INFO_V1(enable_debug_mode);
}

bool debugMode = true;

void print_info_str(const char *str)
{
    ereport(INFO, (errmsg(str)));
}
void print_info(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    ereport(INFO, (errmsg(fmt, args)));
    va_end(args);
}
void print_error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    ereport(ERROR, (errmsg(fmt, args)));
    va_end(args);
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