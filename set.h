#ifndef _CDATASTRUCTS_SET_
#define _CDATASTRUCTS_SET_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "memory.h"
#include "array.h"
#include "hash.h"

#define SET_INVALID_INDEX ((size_t) -1)

/*
	Sets are used to store unique items in an unordered space, given
	that the time complexity to find an item inside it is hopefully
	optimal within O(logN).

	Sets need to be immutable for that reason. Changing values inside
	is forbidden. One can achieve this by adding or removing data from
	them.

	The only function that can potentially break its usage is `set_get`, 
	which allows clients to change their data without permission.
*/

typedef Array SetIndicesBucket;

typedef struct _set_equals {
	void *a;
	void *b;
	size_t n;
} SetEquals;

// One pointer parameter is better than several.
// This will be called a lot of times at once when attempting to find values.
// Keep it as optimal as possible.
typedef bool (*SetEqualsCallback)(SetEquals *);

typedef struct _set {
	Array values;
	Array buckets; // Array<SetIndicesBucket>
	SetEqualsCallback equals;
} Set;

typedef struct _set_iter {
	Set *set;
	size_t bucket;
	size_t offset;
	size_t vindex;
} SetIter;

void set_init(Set *s, size_t vlen, size_t n, SetEqualsCallback callback);
void set_free(Set *s);

void set_reserve(Set *s, size_t n);
void set_resize(Set *s, size_t n);

bool set_add(Set *s, void *x);
bool set_del(Set *s, size_t i);
bool set_pop(Set *s);

void *set_get(Set *s, size_t i);
bool set_read(Set *s, size_t i, void *x);

size_t set_find(Set *s, void *x);
size_t set_find_bucket(Set *s, void *x, size_t *bucket, size_t *index);

SetIter set_iter_first(Set *s);
SetIter set_iter_find(Set *s, void *x);
SetIter set_iter_last(Set *s);

void set_iter_read(SetIter it, void *x);
void *set_iter_get(SetIter it);

void set_iter_next(SetIter *it);
void set_iter_prev(SetIter *it);

bool set_iter_end(SetIter it);
bool set_iter_continue(SetIter it);

size_t set_length(Set *s);

#endif
