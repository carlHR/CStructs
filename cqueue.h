#ifndef _CDATASTRUCTS_CIRCULAR_QUEUE_
#define _CDATASTRUCTS_CIRCULAR_QUEUE_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "memory.h"

#define CQUEUE_INVALID_INDEX ((size_t) -1)
#define CQUEUE_GROWTH ((double) 1.0)
#define CQUEUE_SHRINK ((double) 0.5)
#define CQUEUE_SHRINK_THRESHOLD ((double) 0.25)

typedef struct _cqueue_policy {
	double growth;
	double shrink;
	double shrink_threshold;
	size_t minlen;
} CQueuePolicy;

typedef struct _cqueue {
	uint8_t *data;
	CQueuePolicy policy;
	size_t length;
	size_t count;
	size_t head;
	size_t chunk;
} CQueue;

void cqueue_init(CQueue *q, size_t chunk, size_t minlen);
void cqueue_free(CQueue *q);

void cqueue_resize(CQueue *q, size_t n);

void cqueue_push(CQueue *q, void *x);
void cqueue_pop(CQueue *q);

bool cqueue_top_read(CQueue *q, void *x);
void *cqueue_top_get(CQueue *q);

#endif
