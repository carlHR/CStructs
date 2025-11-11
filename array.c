#include "array.h"

void array_init(Array *a, size_t chunk) {
	a->data = NULL;
	a->length = 0;
	a->capacity = 0;
	a->chunk = chunk;
}

void array_free(Array *a) {
	memory_free(a->data);
	a->data = NULL;
	a->length = 0;
	a->capacity = 0;
}


void array_reserve(Array *a, size_t n) {
	size_t c = 1, ac;
	uint8_t *tmp;

	while (c < n) {
		ac = (size_t) (((double) c) * _ARRAY_GROWTH);

		// increase by a minimum of 2 bytes.
		if (ac <= 1) {
			ac = 2;
		}

		c += ac;
	}

	if (c >= a->capacity) {
		tmp = a->data;
		a->data = memory_malloc(c * a->chunk);
		if (tmp != NULL)
			memcpy(a->data, tmp, a->length * a->chunk);
		memory_free(tmp);
		tmp = NULL;
		a->capacity = c;
	}
}

void array_shrink(Array *a, size_t n) {
	size_t c = 1, ac;
	uint8_t *tmp;

	while (c < n) {
		ac = (size_t) (((double) c) * _ARRAY_GROWTH);

		// increase by a minimum of 2 bytes.
		if (ac <= 1) {
			ac = 2;
		}

		c += ac;
	}

	// Attempts to shrink the array down.
	if (c >= a->length && c < a->capacity) {
		tmp = a->data;
		a->data = memory_malloc(c * a->chunk);
		if (tmp != NULL)
			memcpy(a->data, tmp, a->length * a->chunk);
		memory_free(tmp);
		tmp = NULL;
		a->capacity = c;
	}
}

void array_fit(Array *a) {
	array_resize(a, a->length, NULL);
}

void array_resize(Array *a, size_t n, void *fill) {
	uint8_t *tmp;

	if (n != a->capacity && n != a->length) {
		tmp = a->data;
		a->data = memory_malloc(n * a->chunk);
		a->capacity = n;

		if (n > a->length) {
			if (tmp != NULL)
				memcpy(a->data, tmp, a->length * a->chunk);
			if (fill != NULL) {
				for (size_t i = a->length; i < n; ++i) {
					memmove(&(a->data[i * a->chunk]), fill, a->chunk);
				}
			} else {
				memset(&(a->data[a->length * a->chunk]), 0, (n - a->length) * a->chunk);
			}
		} else if (n <= a->length) {
			if (tmp != NULL)
				memcpy(a->data, tmp, n * a->chunk);
		}

		memory_free(tmp);
		a->length = n;
	}
}

void array_move(Array *a, size_t i, ArrayOperation op, size_t n, void *fill) {
	if (n == 0)
		return;

	switch (op) {
	case ARRAY_MOVE_ADD:
		array_reserve(a, a->length+n);

		if (i < a->length) {
			memmove(&(a->data[a->chunk * (i+n)]), &(a->data[a->chunk * i]), a->chunk * (a->length - i));
			if (fill != NULL) {
				for (size_t k = 0; k < n; ++k) {
					memmove(&(a->data[a->chunk * (i+k)]), fill, a->chunk);
				}
			}
		} else if (i == a->length) {
			if (fill != NULL) {
				for (size_t k = 0; k < n; ++k) {
					memmove(&(a->data[a->chunk * (a->length+k)]), fill, a->chunk);
				}
			}
		}

		a->length += n;

		break;
	case ARRAY_MOVE_DEL:

		// use 'n' as -1 to pop all elements including 'i'.
		if (i < a->length) {
			if (i + n > a->length) {
				n = a->length - i;
			}

			if (i + n < a->length) {
				memmove(&(a->data[a->chunk * (i)]), &(a->data[a->chunk * (i+n)]), a->chunk * (a->length - i));
			}
			
			a->length -= n;
			array_shrink(a, a->length);
		}

		break;
	}
}

void array_insert(Array *a, size_t i, void *x) {
	array_move(a, i, ARRAY_MOVE_ADD, 1, x);
}

void array_erase(Array *a, size_t i) {
	array_move(a, i, ARRAY_MOVE_DEL, 1, NULL);
}

void array_push(Array *a, void *x) {
	array_move(a, a->length, ARRAY_MOVE_ADD, 1, x);
}

void array_pop(Array *a) {
	array_move(a, a->length-1, ARRAY_MOVE_DEL, 1, NULL);
}


void array_set(Array *a, size_t i, void *x) {
	if (i < a->length)
		memmove(&(a->data[i * a->chunk]), x, a->chunk);
}

void array_read(Array *a, size_t i, void *x) {
	if (i < a->length)
		memmove(x, &(a->data[i * a->chunk]), a->chunk);
}

void *array_get(Array *a, size_t i) {
	if (i < a->length)
		return &(a->data[i * a->chunk]);
	else
		return NULL;
}


ArrayIter array_iter_first(Array *a) {
	ArrayIter it;

	it.array = a;
	it.index = 0;

	return it;
}

ArrayIter array_iter_find(Array *a, size_t i) {
	ArrayIter it;

	it.array = a;
	it.index = i;

	return it;
}

ArrayIter array_iter_last(Array *a) {
	ArrayIter it;

	it.array = a;
	it.index = a->length-1;

	return it;
}

ArrayIter array_iter_insert(ArrayIter *it, void *x) {
	ArrayIter nt;
	nt.array = it->array;
	nt.index = it->index;
	if (array_iter_continue(*it)) {
		array_insert(it->array, it->index, x);
		++(it->index);
	} else {
		array_push(it->array, x);
		nt.index = nt.array->length-1;
	}
	return nt;
}

ArrayIter array_iter_erase(ArrayIter *it) {
	ArrayIter nt;
	nt.array = it->array;
	nt.index = it->index;
	if (array_iter_continue(*it)) {
		array_erase(it->array, it->index);
		--(it->index);
	}
	return nt;
}

void array_iter_next(ArrayIter *it) {
	if (it->index < it->array->length) {
		++(it->index);
	}
}

void array_iter_prev(ArrayIter *it) {
	if (it->index < it->array->length) {
		--(it->index);
	}
}

void array_iter_set(ArrayIter it, void *x) {
	array_set(it.array, it.index, x);
}

void array_iter_read(ArrayIter it, void *x) {
	array_read(it.array, it.index, x);
}

void *array_iter_get(ArrayIter it) {
	return array_get(it.array, it.index);
}

bool array_iter_end(ArrayIter it) {
	if (it.index >= it.array->length) {
		return true;
	} else {
		return false;
	}
}

bool array_iter_continue(ArrayIter it) {
	if (it.index < it.array->length) {
		return true;
	} else {
		return false;
	}
}
