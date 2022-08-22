#pragma once 


#ifdef __cplusplus
extern "C" {
#endif

void *getSharedBuffer(size_t size);
void freeBuffer(void *buffer);

#ifdef __cplusplus
}
#endif
