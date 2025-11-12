#ifndef _CDATASTRUCTS_DICT_
#define _CDATASTRUCTS_DICT_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "memory.h"
#include "array.h"
#include "vhashmap.h"

#define DICT_INVALID_INDEX VHASHMAP_INVALID_INDEX

typedef struct _dict {
	Array values;
	VHashmap indices;
} Dict;

typedef struct _dict_iter {
	Dict *dict;
	VHashmapIter vhiter;
	size_t index;
} DictIter;

void dict_init(Dict *d, size_t vlen, size_t n);
void dict_free(Dict *d);

void dict_resize(Dict *d, size_t n);

void dict_set(Dict *d, void *key, size_t klen, void *x);
bool dict_del(Dict *d, void *key, size_t klen);
bool dict_read(Dict *d, void *key, size_t klen, void *x);
void *dict_get(Dict *d, void *key, size_t klen);

DictIter dict_iter_first(Dict *d);
DictIter dict_iter_find(Dict *d, void *key, size_t klen);
DictIter dict_iter_last(Dict *d);

DictIter dict_iter_insert(DictIter *it, void *key, size_t klen, void *x);
DictIter dict_iter_erase(DictIter *it);
void dict_iter_next(DictIter *it);
void dict_iter_prev(DictIter *it);

void dict_iter_set(DictIter it, void *x);
void dict_iter_read(DictIter it, void *x);
void *dict_iter_get(DictIter it);

void dict_iter_read_key(DictIter it, void *key, size_t *klen);
void *dict_iter_get_key(DictIter it, size_t *klen);

bool dict_iter_end(DictIter it);
bool dict_iter_continue(DictIter it);

size_t dict_length(Dict *d);

#endif
