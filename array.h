#ifndef _CDATASTRUCTS_ARRAY_
#define _CDATASTRUCTS_ARRAY_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "memory.h"

#define _ARRAY_GROWTH ((double) 0.33)

/*
	Stores a package of data in each position.

	Remarks:
		array_move is used both to insert and erase items.
			- insert is used to insert on the index.
				- array_move([0 1 2], 0, ARRAY_MOVE_ADD, 2, .) -> [. . 0 1 2]
				- array_move([0 1 2], 1, ARRAY_MOVE_ADD, 2, .) -> [0 . . 1 2]
				- array_move([0 1 2], 2, ARRAY_MOVE_ADD, 2, .) -> [0 1 . . 2]
				- array_move([0 1 2], 3, ARRAY_MOVE_ADD, 2, .) -> [0 1 2 . .]
			- erase is used to remove from the index.
				- array_move([0 1 2], 0, ARRAY_MOVE_ADD, 2, .) -> [2]
				- array_move([0 1 2], 1, ARRAY_MOVE_ADD, 2, .) -> [0]
				- array_move([0 1 2], 2, ARRAY_MOVE_ADD, 2, .) -> [0 1]
				- array_move([0 1 2], 3, ARRAY_MOVE_ADD, 2, .) -> [0 1 2]
*/

typedef struct _array {
	uint8_t *data;
	size_t chunk;
	size_t length;
	size_t capacity;
} Array;

typedef struct _array_iter {
	Array *array;
	size_t index;
} ArrayIter;

typedef enum _array_op {
	ARRAY_MOVE_ADD,
	ARRAY_MOVE_DEL
} ArrayOperation;

void array_init(Array *a, size_t chunk);
void array_free(Array *a);

void array_reserve(Array *a, size_t n);
void array_shrink(Array *a, size_t n);
void array_fit(Array *a);
void array_resize(Array *a, size_t n, void *fill);
void array_move(Array *a, size_t i, ArrayOperation op, size_t n, void *fill);

void array_insert(Array *a, size_t i, void *x);
void array_erase(Array *a, size_t i);
void array_push(Array *a, void *x);
void array_pop(Array *a);

void array_set(Array *a, size_t i, void *x);
void array_read(Array *a, size_t i, void *x);
void *array_get(Array *a, size_t i);

ArrayIter array_iter_first(Array *a);
ArrayIter array_iter_find(Array *a, size_t i);
ArrayIter array_iter_last(Array *a);
ArrayIter array_iter_insert(ArrayIter *it, void *x);
ArrayIter array_iter_erase(ArrayIter *it);
void array_iter_next(ArrayIter *it);
void array_iter_prev(ArrayIter *it);
void array_iter_set(ArrayIter it, void *x);
void array_iter_read(ArrayIter it, void *x);
void *array_iter_get(ArrayIter it);
bool array_iter_end(ArrayIter it);
bool array_iter_continue(ArrayIter it);

#endif
