#ifndef _CDATASTRUCTS_VCACHE_
#define _CDATASTRUCTS_VCACHE_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "memory.h"
#include "array.h"
#include "varray.h"
#include "hashmap.h"

#define VCACHE_INVALID_INDEX HASHMAP_INVALID_INDEX

typedef struct _cache {
	VArray values;
	Hashmap indices;
} VCache;

typedef struct _vcache_iter {
	VCache *vcache;
	HashmapIter hiter;
	size_t index;
} VCacheIter;

void vcache_init(VCache *c, size_t klen, size_t n);
void vcache_free(VCache *c);

void vcache_resize(VCache *c, size_t n);

void vcache_set(VCache *c, void *key, void *x, size_t vlen);
bool vcache_del(VCache *c, void *key);
bool vcache_read(VCache *c, void *key, void *x, size_t *vlen);
void *vcache_get(VCache *c, void *key, size_t *vlen);

VCacheIter vcache_iter_first(VCache *c);
VCacheIter vcache_iter_find(VCache *c, void *key);
VCacheIter vcache_iter_last(VCache *c);

VCacheIter vcache_iter_insert(VCacheIter *it, void *key, void *x, size_t vlen);
VCacheIter vcache_iter_erase(VCacheIter *it);
void vcache_iter_next(VCacheIter *it);
void vcache_iter_prev(VCacheIter *it);

void vcache_iter_set(VCacheIter it, void *x, size_t vlen);
void vcache_iter_read(VCacheIter it, void *x, size_t *vlen);
void *vcache_iter_get(VCacheIter it, size_t *vlen);

void vcache_iter_read_key(VCacheIter it, void *key);
void *vcache_iter_get_key(VCacheIter it);

bool vcache_iter_end(VCacheIter it);
bool vcache_iter_continue(VCacheIter it);

size_t vcache_length(VCache *c);

#endif
