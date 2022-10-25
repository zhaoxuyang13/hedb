#pragma once

#ifdef ENABLE_TEST_OPS
#define print_info(...) \
            printf(__VA_ARGS__)
#define print_error(...) \
            printf(__VA_ARGS__)
#else
#define print_info(...) \
            ereport(INFO, (errmsg(__VA_ARGS__)))
#define print_error(...) \
            ereport(ERROR, (errmsg(__VA_ARGS__)))
#endif