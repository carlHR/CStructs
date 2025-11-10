#ifndef _CDATASTRUCTS_VHASHMAP_
#define _CDATASTRUCTS_VHASHMAP_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "memory.h"
#include "varray.h"
#include "hash.h"

#define VHASHMAP_INVALID_INDEX ((size_t) -1)

typedef VArray VHashmapBucket;

typedef struct _vhashmap {
	Array buckets; // Array<VHashmapBucket>
	size_t vlen;
} VHashmap;

typedef struct _vhashmap_iter {
	VHashmap *vhashmap;
	size_t bucket;
	size_t index;
} VHashmapIter;

bool vhashmap_find(VHashmap *h, void *key, size_t klen, VHashmapBucket **bucket, size_t *bindex, size_t *index);

void vhashmap_init(VHashmap *h, size_t vlen, size_t n);
void vhashmap_free(VHashmap *h);

void vhashmap_resize(VHashmap *h, size_t n);

void vhashmap_set(VHashmap *h, void *key, size_t klen, void *x);
bool vhashmap_del(VHashmap *h, void *key, size_t klen);
bool vhashmap_read(VHashmap *h, void *key, size_t klen, void *x);
void *vhashmap_get(VHashmap *h, void *key, size_t klen);

VHashmapIter vhashmap_iter_first(VHashmap *h);
VHashmapIter vhashmap_iter_find(VHashmap *h, void *key, size_t klen);
VHashmapIter vhashmap_iter_last(VHashmap *h);

VHashmapIter vhashmap_iter_insert(VHashmapIter *it, void *key, size_t klen, void *x);
VHashmapIter vhashmap_iter_erase(VHashmapIter *it);
void vhashmap_iter_next(VHashmapIter *it);
void vhashmap_iter_prev(VHashmapIter *it);

void vhashmap_iter_set(VHashmapIter it, void *x);
void vhashmap_iter_read(VHashmapIter it, void *x);
void *vhashmap_iter_get(VHashmapIter it);

void vhashmap_iter_read_key(VHashmapIter it, void *key, size_t *klen);
void *vhashmap_iter_get_key(VHashmapIter it, size_t *klen);

bool vhashmap_iter_end(VHashmapIter it);
bool vhashmap_iter_continue(VHashmapIter it);

#endif
