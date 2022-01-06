#pragma once
#include <stddef.h> // for size_t


/* get shared buffer for asynchronous call, return buffer pointer */
void *getSharedBuffer(size_t size);
/* free shared buffer. */
void *freeBuffer(void *buffer);
