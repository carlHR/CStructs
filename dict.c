#include "dict.h"

void dict_init(Dict *d, size_t vlen, size_t n) {
	array_init(&(d->values), vlen);
	vhashmap_init(&(d->indices), sizeof(size_t), n);
}

void dict_free(Dict *d) {
	vhashmap_free(&(d->indices));
	array_free(&(d->values));
}


void dict_resize(Dict *d, size_t n) {
	vhashmap_resize(&(d->indices), n);
}


void dict_set(Dict *d, void *key, size_t klen, void *x) {
	VHashmapBucket *bucket;
	uint8_t *data;
	size_t bindex;
	size_t index;
	size_t vindex;

	if (vhashmap_find(&(d->indices), key, klen, &bucket, &bindex, &index)) {
		data = varray_get(bucket, index, NULL);
		memmove(&vindex, &(data[klen]), sizeof(size_t));
		array_set(&(d->values), vindex, x);
	} else {
		vindex = d->values.length;
		array_push(&(d->values), x);
		vhashmap_set(&(d->indices), key, klen, &vindex);
	}
}

bool dict_del(Dict *d, void *key, size_t klen) {
	VHashmapBucket *bucket;
	uint8_t *data;
	size_t bindex;
	size_t index;
	size_t vindex;

	if (vhashmap_find(&(d->indices), key, klen, &bucket, &bindex, &index)) {
		data = varray_get(bucket, index, NULL);
		memmove(&vindex, &(data[klen]), sizeof(size_t));

		varray_erase(bucket, index);
		array_erase(&(d->values), vindex);

		// When an vindex is removed, must update all others
		for (VHashmapIter it = vhashmap_iter_first(&(d->indices)); !vhashmap_iter_end(it); vhashmap_iter_next(&it)) {
			vhashmap_iter_read(it, &index);
			if (vindex < index) {
				--index;
				vhashmap_iter_set(it, &index);
			}
		}

		return true;
	}

	return false;
}

bool dict_read(Dict *d, void *key, size_t klen, void *x) {
	VHashmapBucket *bucket;
	uint8_t *data;
	size_t bindex;
	size_t index;
	size_t vindex;

	if (vhashmap_find(&(d->indices), key, klen, &bucket, &bindex, &index)) {
		data = varray_get(bucket, index, NULL);
		memmove(&vindex, &(data[klen]), sizeof(size_t));
		array_read(&(d->values), vindex, x);
		return true;
	} else {
		return false;
	}
}

void *dict_get(Dict *d, void *key, size_t klen) {
	VHashmapBucket *bucket;
	uint8_t *data;
	size_t bindex;
	size_t index;
	size_t vindex;

	if (vhashmap_find(&(d->indices), key, klen, &bucket, &bindex, &index)) {
		data = varray_get(bucket, index, NULL);
		memmove(&vindex, &(data[klen]), sizeof(size_t));
		return array_get(&(d->values), vindex);
	} else {
		return NULL;
	}
}


DictIter dict_iter_first(Dict *d) {
	DictIter it;

	it.dict = d;
	it.vhiter = vhashmap_iter_first(&(d->indices));
	vhashmap_iter_read(it.vhiter, &(it.index));

	return it;
}

DictIter dict_iter_find(Dict *d, void *key, size_t klen) {
	DictIter it;

	it.dict = d;
	it.vhiter = vhashmap_iter_find(&(d->indices), key, klen);
	vhashmap_iter_read(it.vhiter, &(it.index));

	return it;
}

DictIter dict_iter_last(Dict *d) {
	DictIter it;

	it.dict = d;
	it.vhiter = vhashmap_iter_last(&(d->indices));
	vhashmap_iter_read(it.vhiter, &(it.index));

	return it;
}


DictIter dict_iter_insert(DictIter *it, void *key, size_t klen, void *x) {
	DictIter nt;

	nt.dict = it->dict;
	nt.vhiter = vhashmap_iter_insert(&(it->vhiter), key, klen, NULL);
	nt.index = DICT_INVALID_INDEX;

	// If the element was inserted, set index and x.
	if (vhashmap_iter_continue(nt.vhiter)) {
		if (nt.vhiter.bucket != it->vhiter.bucket || nt.vhiter.index != it->vhiter.index) {
			nt.index = it->dict->values.length;
			array_push(&(it->dict->values), x);
			memmove(vhashmap_iter_get(nt.vhiter), &(nt.index), sizeof(size_t));
		}
	}

	vhashmap_iter_read(nt.vhiter, &(nt.index));
	vhashmap_iter_read(it->vhiter, &(it->index));

	return nt;
}

DictIter dict_iter_erase(DictIter *it) {
	DictIter nt;
	size_t index;

	nt.dict = it->dict;
	nt.index = DICT_INVALID_INDEX;

	if (dict_iter_continue(*it)) {
		array_erase(&(it->dict->values), it->index);
		nt.vhiter = vhashmap_iter_erase(&(it->vhiter));

		// When an vindex is removed, must update all others
		for (VHashmapIter ot = vhashmap_iter_first(&(it->dict->indices)); !vhashmap_iter_end(ot); vhashmap_iter_next(&ot)) {
			vhashmap_iter_read(ot, &index);
			if (it->index < index) {
				--index;
				vhashmap_iter_set(ot, &index);
			}
		}

		vhashmap_iter_read(nt.vhiter, &(nt.index));
		vhashmap_iter_read(it->vhiter, &(it->index));
	}

	return nt;
}

void dict_iter_next(DictIter *it) {
	vhashmap_iter_next(&(it->vhiter));
	if (vhashmap_iter_continue(it->vhiter)) {
		vhashmap_iter_read(it->vhiter, &(it->index));
	} else {
		it->index = DICT_INVALID_INDEX;
	}
}

void dict_iter_prev(DictIter *it) {
	vhashmap_iter_prev(&(it->vhiter));
	if (vhashmap_iter_continue(it->vhiter)) {
		vhashmap_iter_read(it->vhiter, &(it->index));
	} else {
		it->index = DICT_INVALID_INDEX;
	}
}


void dict_iter_set(DictIter it, void *x) {
	if (dict_iter_continue(it)) {
		array_set(&(it.dict->values), it.index, x);
	}
}

void dict_iter_read(DictIter it, void *x) {
	if (dict_iter_continue(it)) {
		array_read(&(it.dict->values), it.index, x);
	}
}

void *dict_iter_get(DictIter it) {
	if (dict_iter_continue(it)) {
		return array_get(&(it.dict->values), it.index);
	} else {
		return NULL;
	}
}


void dict_iter_read_key(DictIter it, void *key, size_t *klen) {
	vhashmap_iter_read_key(it.vhiter, key, klen);
}

void *dict_iter_get_key(DictIter it, size_t *klen) {
	return vhashmap_iter_get_key(it.vhiter, klen);
}


bool dict_iter_end(DictIter it) {
	if (vhashmap_iter_end(it.vhiter) || it.index >= it.dict->values.length) {
		return true;
	} else {
		return false;
	}
}

bool dict_iter_continue(DictIter it) {
	if (vhashmap_iter_end(it.vhiter) || it.index >= it.dict->values.length) {
		return false;
	} else {
		return true;
	}
}

size_t dict_length(Dict *d) {
	return d->values.length;
}
