#ifndef _CDATASTRUCTS_MEMORY_
#define _CDATASTRUCTS_MEMORY_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void * (*MemoryRecoveryCallback)(size_t);

void *memory_malloc(size_t n);
void memory_free(void *p);
void memory_set_recovery_callback(MemoryRecoveryCallback callback);

#endif
