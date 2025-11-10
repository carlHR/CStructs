#ifndef _CDATASTRUCTS_VARRAY_
#define _CDATASTRUCTS_VARRAY_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "array.h"

/*
	VArray is supposed to mean Variadic Array, or simply, an array that can store N
	items of different sizes inside it. The array is supposed to be continuous just
	like "array.h".

	VArray uses two arrays in order to ensure constant time complexity in order to
	index items. However, as a drawback of not being able to identify "empty" or "vacant"
	spaces within the array, there's no similar function to array_move for varrays.

	If you plan to store all items with the same size in bytes, use "array.h". Its
	cheaper and faster.
*/

typedef struct _varray_metadata {
	size_t offset;
	size_t length;
} VArrayMetadata;

typedef struct _varray {
	Array data;
	Array metadata;
} VArray;

typedef struct _varray_iter {
	VArray *varray;
	ArrayIter mditer;
} VArrayIter;

void varray_init(VArray *v);
void varray_free(VArray *v);

void varray_reserve(VArray *v, size_t nitems, size_t datalen);
void varray_shrink(VArray *v, size_t nitems, size_t datalen);
void varray_fit(VArray *v, size_t nitems, size_t datalen);

void varray_push(VArray *v, void *x, size_t len);
void varray_pop(VArray *v);
void varray_insert(VArray *v, size_t i, void *x, size_t len);
void varray_erase(VArray *v, size_t i);
void varray_read(VArray *v, size_t i, void *x, size_t *len);
void varray_set(VArray *v, size_t i, void *x, size_t newlen);
void *varray_get(VArray *v, size_t i, size_t *len);

VArrayIter varray_iter_first(VArray *v);
VArrayIter varray_iter_last(VArray *v);
VArrayIter varray_iter_find(VArray *v, size_t index);

bool varray_iter_end(VArrayIter it);
bool varray_iter_continue(VArrayIter it);

void varray_iter_next(VArrayIter *it);
void varray_iter_prev(VArrayIter *it);

VArrayIter varray_iter_insert(VArrayIter *it, void *x, size_t len);
VArrayIter varray_iter_erase(VArrayIter *it);
void varray_iter_set(VArrayIter it, void *x, size_t newlen);
void varray_iter_read(VArrayIter it, void *x, size_t *len);
void *varray_iter_get(VArrayIter it, size_t *len);

#endif
