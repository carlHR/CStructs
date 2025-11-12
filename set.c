#include "set.h"

static bool set_default_equals_callback(SetEquals *eq) {
	return (memcmp(eq->a, eq->b, eq->n) == 0);
}

void set_init(Set *s, size_t vlen, size_t n, SetEqualsCallback callback) {
	SetIndicesBucket bucket;
	array_init(&(s->values), vlen);
	array_init(&(s->buckets), sizeof(SetIndicesBucket));
	for (size_t i = 0; i < n; ++i) {
		array_init(&bucket, sizeof(size_t));
		array_push(&(s->buckets), &bucket);
	}
	if (callback == NULL) {
		s->equals = set_default_equals_callback;
	} else {
		s->equals = callback;
	}
}

void set_free(Set *s) {
	SetIndicesBucket *bucket;
	for (ArrayIter it = array_iter_first(&(s->buckets)); !array_iter_end(it); array_iter_next(&it)) {
		bucket = array_iter_get(it);
		array_free(bucket);
	}
	array_free(&(s->values));
	array_free(&(s->buckets));
}


void set_reserve(Set *s, size_t n) {
	array_reserve(&(s->values), n);
}

void set_resize(Set *s, size_t n) {
	Set tmp;

	set_init(&tmp, s->values.chunk, n, s->equals);
	for (SetIter it = set_iter_first(s); !set_iter_end(it); set_iter_next(&it)) {
		set_add(&tmp, set_iter_get(it));
	}
	set_free(s);
	*s = tmp;
}


bool set_add(Set *s, void *x) {
	SetIndicesBucket *bucket;
	size_t bindex;

	if (set_find_bucket(s, x, &bindex, NULL) == SET_INVALID_INDEX) {
		bucket = array_get(&(s->buckets), bindex);

		array_push(bucket, &(s->values.length));
		array_push(&(s->values), x);

		return true;
	} else {
		return false;
	}
}

bool set_del(Set *s, size_t i) {
	SetIndicesBucket *bucket;
	size_t bindex;
	size_t offset;
	size_t vindex;
	size_t idx;

	vindex = set_find_bucket(s, array_get(&(s->values), i), &bindex, &offset);

	if (vindex != SET_INVALID_INDEX) {
		for (SetIter it = set_iter_first(s); !set_iter_end(it); set_iter_next(&it)) {
			bucket = array_get(&(s->buckets), it.bucket);
			array_read(bucket, it.offset, &idx);

			if (vindex < idx) {
				--idx;
				array_set(bucket, it.offset, &idx);
			}
		}

		bucket = array_get(&(s->buckets), bindex);
		array_erase(bucket, offset);
		array_erase(&(s->values), vindex);

		return true;
	} else {
		return false;
	}
}

bool set_pop(Set *s) {
	return set_del(s, s->values.length-1);
}

void *set_get(Set *s, size_t i) {
	return array_get(&(s->values), i);
}

bool set_read(Set *s, size_t i, void *x) {
	if (i < s->values.length) {
		array_read(&(s->values), i, x);
		return true;
	} else {
		return false;
	}
}

size_t set_find(Set *s, void *x) {
	SetIndicesBucket *bucket;
	SetEquals eq;
	size_t idx;

	size_t hash;
	size_t vindex;

	hash = (size_t) (hash_buf_length(x, s->values.chunk) % ((HashPrecision) s->buckets.length));
	bucket = array_get(&(s->buckets), hash);

	for (ArrayIter it = array_iter_first(bucket); !array_iter_end(it); array_iter_next(&it)) {
		array_iter_read(it, &idx);
		eq.a = x;
		eq.b = array_get(&(s->values), idx);
		eq.n = s->values.chunk;
		if (s->equals(&eq)) {
			return idx;
		}
	}

	return SET_INVALID_INDEX;
}

size_t set_find_bucket(Set *s, void *x, size_t *_bucket, size_t *_index) {
	SetIndicesBucket *bucket;
	SetEquals eq;
	size_t idx;

	size_t hash;
	size_t vindex;

	hash = (size_t) (hash_buf_length(x, s->values.chunk) % ((HashPrecision) s->buckets.length));
	bucket = array_get(&(s->buckets), hash);

	if (_bucket != NULL) {
		*_bucket = hash;
	}

	for (ArrayIter it = array_iter_first(bucket); !array_iter_end(it); array_iter_next(&it)) {
		array_iter_read(it, &idx);
		eq.a = x;
		eq.b = array_get(&(s->values), idx);
		eq.n = s->values.chunk;
		if (s->equals(&eq)) {
			if (_index != NULL) {
				*_index = it.index;
			}
			return idx;
		}
	}

	return SET_INVALID_INDEX;
}


SetIter set_iter_first(Set *s) {
	SetIndicesBucket *bucket;
	SetIter it;

	it.set = s;
	it.bucket = SET_INVALID_INDEX;
	it.offset = SET_INVALID_INDEX;
	it.vindex = SET_INVALID_INDEX;

	for (ArrayIter jt = array_iter_first(&(s->buckets)); !array_iter_end(jt); array_iter_next(&jt)) {
		bucket = array_iter_get(jt);
		if (bucket->length > 0) {
			it.bucket = jt.index;
			it.offset = 0;
			array_read(bucket, it.offset, &(it.vindex));
			break;
		}
	}

	return it;
}

SetIter set_iter_find(Set *s, void *x) {
	SetIndicesBucket *bucket;
	SetIter it;

	it.set = s;

	if (set_find_bucket(s, x, &(it.bucket), &(it.offset))) {
		bucket = array_get(&(s->buckets), it.bucket);
		array_read(bucket, it.offset, &(it.vindex));
	} else {
		it.bucket = SET_INVALID_INDEX;
		it.offset = SET_INVALID_INDEX;
		it.vindex = SET_INVALID_INDEX;
	}

	return it;
}

SetIter set_iter_last(Set *s) {
	SetIndicesBucket *bucket;
	SetIter it;

	it.set = s;
	it.bucket = SET_INVALID_INDEX;
	it.offset = SET_INVALID_INDEX;
	it.vindex = SET_INVALID_INDEX;

	for (ArrayIter jt = array_iter_last(&(s->buckets)); !array_iter_end(jt); array_iter_prev(&jt)) {
		bucket = array_iter_get(jt);
		if (bucket->length > 0) {
			it.bucket = jt.index;
			it.offset = bucket->length-1;
			array_read(bucket, it.offset, &(it.vindex));
			break;
		}
	}

	return it;
}

void set_iter_read(SetIter it, void *x) {
	if (set_iter_continue(it)) {
		array_read(&(it.set->values), it.vindex, x);
	}
}

void *set_iter_get(SetIter it) {
	if (set_iter_continue(it)) {
		return array_get(&(it.set->values), it.vindex);
	} else {
		return NULL;
	}
}

void set_iter_next(SetIter *it) {
	SetIndicesBucket *bucket;
	if (set_iter_continue(*it)) {
		bucket = array_get(&(it->set->buckets), it->bucket);
		++(it->offset);
		if (it->offset >= bucket->length) {
			for (ArrayIter jt = array_iter_find(&(it->set->buckets), (it->bucket)+1); !array_iter_end(jt); array_iter_next(&jt)) {
				bucket = array_iter_get(jt);
				if (bucket->length > 0) {
					it->bucket = jt.index;
					it->offset = 0;
					array_read(bucket, it->offset, &(it->vindex));
					return;
				}
			}
			it->offset = SET_INVALID_INDEX;
			it->bucket = SET_INVALID_INDEX;
			it->vindex = SET_INVALID_INDEX;
		} else {
			array_read(bucket, it->offset, &(it->vindex));
		}
	}
}

void set_iter_prev(SetIter *it) {
	SetIndicesBucket *bucket;
	if (set_iter_continue(*it)) {
		bucket = array_get(&(it->set->buckets), it->bucket);
		--(it->offset);
		if (it->offset >= bucket->length) {
			for (ArrayIter jt = array_iter_find(&(it->set->buckets), (it->bucket)-1); !array_iter_end(jt); array_iter_prev(&jt)) {
				bucket = array_iter_get(jt);
				if (bucket->length > 0) {
					it->bucket = jt.index;
					it->offset = bucket->length-1;
					array_read(bucket, it->offset, &(it->vindex));
					return;
				}
			}
			it->offset = SET_INVALID_INDEX;
			it->bucket = SET_INVALID_INDEX;
			it->vindex = SET_INVALID_INDEX;
		} else {
			array_read(bucket, it->offset, &(it->vindex));
		}
	}
}

bool set_iter_end(SetIter it) {
	SetIndicesBucket *bucket;
	if ((it.vindex >= it.set->values.length) || (it.bucket >= it.set->buckets.length)) {
		return true;
	} else {
		bucket = array_get(&(it.set->buckets), it.bucket);
		if (it.offset >= bucket->length) {
			return true;
		} else {
			return false;
		}
	}
}

bool set_iter_continue(SetIter it) {
	SetIndicesBucket *bucket;
	if ((it.vindex >= it.set->values.length) || (it.bucket >= it.set->buckets.length)) {
		return false;
	} else {
		bucket = array_get(&(it.set->buckets), it.bucket);
		if (it.offset >= bucket->length) {
			return false;
		} else {
			return true;
		}
	}
}

size_t set_length(Set *s) {
	return s->values.length;
}
