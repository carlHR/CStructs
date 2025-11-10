#ifndef _CDATASTRUCTS_HASHMAP_
#define _CDATASTRUCTS_HASHMAP_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "memory.h"
#include "array.h"
#include "hash.h"

#define HASHMAP_INVALID_INDEX ((size_t) -1)

typedef Array HashmapBucket;

typedef struct _hashmap {
	Array buckets; // Array<HashmapBucket>
	size_t klen;
	size_t vlen;
} Hashmap;

typedef struct _hashmap_iter {
	Hashmap *hashmap;
	size_t bucket;
	size_t index;
} HashmapIter;

bool hashmap_find(Hashmap *h, void *key, HashmapBucket **bucket, size_t *bindex, size_t *index);

void hashmap_init(Hashmap *h, size_t klen, size_t vlen, size_t n);
void hashmap_free(Hashmap *h);

void hashmap_resize(Hashmap *h, size_t n);

void hashmap_set(Hashmap *h, void *key, void *x);
bool hashmap_del(Hashmap *h, void *key);
bool hashmap_read(Hashmap *h, void *key, void *x);
void *hashmap_get(Hashmap *h, void *key);

HashmapIter hashmap_iter_first(Hashmap *h);
HashmapIter hashmap_iter_find(Hashmap *h, void *key);
HashmapIter hashmap_iter_last(Hashmap *h);

HashmapIter hashmap_iter_insert(HashmapIter *it, void *key, void *x);
HashmapIter hashmap_iter_erase(HashmapIter *it);
void hashmap_iter_next(HashmapIter *it);
void hashmap_iter_prev(HashmapIter *it);

void hashmap_iter_set(HashmapIter it, void *x);
void hashmap_iter_read(HashmapIter it, void *x);
void *hashmap_iter_get(HashmapIter it);

void hashmap_iter_read_key(HashmapIter it, void *key);
void *hashmap_iter_get_key(HashmapIter it);

bool hashmap_iter_end(HashmapIter it);
bool hashmap_iter_continue(HashmapIter it);

#endif
