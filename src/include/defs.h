#pragma once 

typedef unsigned char BYTE; // 1byte
typedef unsigned short WORD; // 2bytes
typedef unsigned long DWORD; //4bytes

#define BULK_SIZE 128
#define KV_LENGTH sizeof(uint64_t)
#define INT32_LENGTH sizeof(int)
#define FLOAT4_LENGTH sizeof(float)
#define TIMESTAMP int64_t
#define TIMESTAMP_LENGTH sizeof(TIMESTAMP)
#define STRING_LENGTH 1024

