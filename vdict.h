#ifndef _CDATASTRUCTS_VDICT_
#define _CDATASTRUCTS_VDICT_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "memory.h"
#include "array.h"
#include "varray.h"
#include "vhashmap.h"

#define VDICT_INVALID_INDEX VHASHMAP_INVALID_INDEX

typedef struct _vdict {
	VArray values;
	VHashmap indices;
} VDict;

typedef struct _vdict_iter {
	VDict *vdict;
	VHashmapIter vhiter;
	size_t index;
} VDictIter;

void vdict_init(VDict *d, size_t n);
void vdict_free(VDict *d);

void vdict_resize(VDict *d, size_t n);

void vdict_set(VDict *d, void *key, size_t klen, void *x, size_t vlen);
bool vdict_del(VDict *d, void *key, size_t klen);
bool vdict_read(VDict *d, void *key, size_t klen, void *x, size_t *vlen);
void *vdict_get(VDict *d, void *key, size_t klen, size_t *vlen);

VDictIter vdict_iter_first(VDict *d);
VDictIter vdict_iter_find(VDict *d, void *key, size_t klen);
VDictIter vdict_iter_last(VDict *d);

VDictIter vdict_iter_insert(VDictIter *it, void *key, size_t klen, void *x, size_t vlen);
VDictIter vdict_iter_erase(VDictIter *it);
void vdict_iter_next(VDictIter *it);
void vdict_iter_prev(VDictIter *it);

void vdict_iter_set(VDictIter it, void *x, size_t vlen);
void vdict_iter_read(VDictIter it, void *x, size_t *vlen);
void *vdict_iter_get(VDictIter it, size_t *vlen);

void vdict_iter_read_key(VDictIter it, void *key, size_t *klen);
void *vdict_iter_get_key(VDictIter it, size_t *klen);

bool vdict_iter_end(VDictIter it);
bool vdict_iter_continue(VDictIter it);

size_t vdict_length(VDict *d);

#endif
