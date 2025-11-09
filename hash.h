#ifndef _CDATASTRUCTS_HASH_
#define _CDATASTRUCTS_HASH_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
	
typedef unsigned long HashPrecision;
HashPrecision hash_buf_length(void *buf, size_t length);

#endif
