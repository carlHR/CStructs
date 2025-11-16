#ifndef _CDATASTRUCTS_CACHE_
#define _CDATASTRUCTS_CACHE_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "memory.h"
#include "array.h"
#include "hashmap.h"

#define CACHE_INVALID_INDEX HASHMAP_INVALID_INDEX

typedef struct _cache {
	Array values;
	Hashmap indices;
} Cache;

typedef struct _cache_iter {
	Cache *cache;
	HashmapIter hiter;
	size_t index;
} CacheIter;

void cache_init(Cache *c, size_t klen, size_t vlen, size_t n);
void cache_free(Cache *c);

void cache_resize(Cache *c, size_t n);

void cache_set(Cache *c, void *key, void *x);
bool cache_del(Cache *c, void *key);
bool cache_read(Cache *c, void *key, void *x);
void *cache_get(Cache *c, void *key);
size_t cache_find(Cache *c, void *key);

CacheIter cache_iter_first(Cache *c);
CacheIter cache_iter_find(Cache *c, void *key);
CacheIter cache_iter_last(Cache *c);

CacheIter cache_iter_insert(CacheIter *it, void *key, void *x);
CacheIter cache_iter_erase(CacheIter *it);
void cache_iter_next(CacheIter *it);
void cache_iter_prev(CacheIter *it);

void cache_iter_set(CacheIter it, void *x);
void cache_iter_read(CacheIter it, void *x);
void *cache_iter_get(CacheIter it);

void cache_iter_read_key(CacheIter it, void *key);
void *cache_iter_get_key(CacheIter it);

bool cache_iter_end(CacheIter it);
bool cache_iter_continue(CacheIter it);

size_t cache_length(Cache *c);

#endif
