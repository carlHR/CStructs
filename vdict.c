#include "vdict.h"

void vdict_init(VDict *d, size_t n) {
	varray_init(&(d->values));
	vhashmap_init(&(d->indices), sizeof(size_t), n);
}

void vdict_free(VDict *d) {
	vhashmap_free(&(d->indices));
	varray_free(&(d->values));
}


void vdict_resize(VDict *d, size_t n) {
	vhashmap_resize(&(d->indices), n);
}


void vdict_set(VDict *d, void *key, size_t klen, void *x, size_t vlen) {
	VHashmapBucket *bucket;
	uint8_t *data;
	size_t bindex;
	size_t index;
	size_t vindex;

	if (vhashmap_find(&(d->indices), key, klen, &bucket, &bindex, &index)) {
		data = varray_get(bucket, index, NULL);
		memmove(&vindex, &(data[klen]), sizeof(size_t));
		varray_set(&(d->values), vindex, x, vlen);
	} else {
		vindex = d->values.metadata.length;
		varray_push(&(d->values), x, vlen);
		vhashmap_set(&(d->indices), key, klen, &vindex);
	}
}

bool vdict_del(VDict *d, void *key, size_t klen) {
	VHashmapBucket *bucket;
	uint8_t *data;
	size_t bindex;
	size_t index;
	size_t vindex;

	if (vhashmap_find(&(d->indices), key, klen, &bucket, &bindex, &index)) {
		data = varray_get(bucket, index, NULL);
		memmove(&vindex, &(data[klen]), sizeof(size_t));

		varray_erase(bucket, index);
		varray_erase(&(d->values), vindex);

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

bool vdict_read(VDict *d, void *key, size_t klen, void *x, size_t *vlen) {
	VHashmapBucket *bucket;
	uint8_t *data;
	size_t bindex;
	size_t index;
	size_t vindex;

	if (vhashmap_find(&(d->indices), key, klen, &bucket, &bindex, &index)) {
		data = varray_get(bucket, index, NULL);
		memmove(&vindex, &(data[klen]), sizeof(size_t));
		varray_read(&(d->values), vindex, x, vlen);
		return true;
	} else {
		return false;
	}
}

void *vdict_get(VDict *d, void *key, size_t klen, size_t *vlen) {
	VHashmapBucket *bucket;
	uint8_t *data;
	size_t bindex;
	size_t index;
	size_t vindex;

	if (vhashmap_find(&(d->indices), key, klen, &bucket, &bindex, &index)) {
		data = varray_get(bucket, index, NULL);
		memmove(&vindex, &(data[klen]), sizeof(size_t));
		return varray_get(&(d->values), vindex, vlen);
	} else {
		return NULL;
	}
}


VDictIter vdict_iter_first(VDict *d) {
	VDictIter it;

	it.vdict = d;
	it.vhiter = vhashmap_iter_first(&(d->indices));
	vhashmap_iter_read(it.vhiter, &(it.index));

	return it;
}

VDictIter vdict_iter_find(VDict *d, void *key, size_t klen) {
	VDictIter it;

	it.vdict = d;
	it.vhiter = vhashmap_iter_find(&(d->indices), key, klen);
	vhashmap_iter_read(it.vhiter, &(it.index));

	return it;
}

VDictIter vdict_iter_last(VDict *d) {
	VDictIter it;

	it.vdict = d;
	it.vhiter = vhashmap_iter_last(&(d->indices));
	vhashmap_iter_read(it.vhiter, &(it.index));

	return it;
}


VDictIter vdict_iter_insert(VDictIter *it, void *key, size_t klen, void *x, size_t vlen) {
	VDictIter nt;

	nt.vdict = it->vdict;
	nt.vhiter = vhashmap_iter_insert(&(it->vhiter), key, klen, NULL);
	nt.index = VDICT_INVALID_INDEX;

	// If the element was inserted, set index and x.
	if (vhashmap_iter_continue(nt.vhiter)) {
		if (nt.vhiter.bucket != it->vhiter.bucket || nt.vhiter.index != it->vhiter.index) {
			nt.index = it->vdict->values.metadata.length;
			varray_push(&(it->vdict->values), x, vlen);
			memmove(vhashmap_iter_get(nt.vhiter), &(nt.index), sizeof(size_t));
		}
	}

	vhashmap_iter_read(nt.vhiter, &(nt.index));
	vhashmap_iter_read(it->vhiter, &(it->index));

	return nt;
}

VDictIter vdict_iter_erase(VDictIter *it) {
	VDictIter nt;
	size_t index;

	nt.vdict = it->vdict;
	nt.index = VDICT_INVALID_INDEX;

	if (vdict_iter_continue(*it)) {
		varray_erase(&(it->vdict->values), it->index);
		nt.vhiter = vhashmap_iter_erase(&(it->vhiter));

		// When an vindex is removed, must update all others
		for (VHashmapIter ot = vhashmap_iter_first(&(it->vdict->indices)); !vhashmap_iter_end(ot); vhashmap_iter_next(&ot)) {
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

void vdict_iter_next(VDictIter *it) {
	vhashmap_iter_next(&(it->vhiter));
	if (vhashmap_iter_continue(it->vhiter)) {
		vhashmap_iter_read(it->vhiter, &(it->index));
	} else {
		it->index = VDICT_INVALID_INDEX;
	}
}

void vdict_iter_prev(VDictIter *it) {
	vhashmap_iter_prev(&(it->vhiter));
	if (vhashmap_iter_continue(it->vhiter)) {
		vhashmap_iter_read(it->vhiter, &(it->index));
	} else {
		it->index = VDICT_INVALID_INDEX;
	}
}


void vdict_iter_set(VDictIter it, void *x, size_t vlen) {
	if (vdict_iter_continue(it)) {
		varray_set(&(it.vdict->values), it.index, x, vlen);
	}
}

void vdict_iter_read(VDictIter it, void *x, size_t *vlen) {
	if (vdict_iter_continue(it)) {
		varray_read(&(it.vdict->values), it.index, x, vlen);
	}
}

void *vdict_iter_get(VDictIter it, size_t *vlen) {
	if (vdict_iter_continue(it)) {
		return varray_get(&(it.vdict->values), it.index, vlen);
	} else {
		return NULL;
	}
}


void vdict_iter_read_key(VDictIter it, void *key, size_t *klen) {
	vhashmap_iter_read_key(it.vhiter, key, klen);
}

void *vdict_iter_get_key(VDictIter it, size_t *klen) {
	return vhashmap_iter_get_key(it.vhiter, klen);
}


bool vdict_iter_end(VDictIter it) {
	if (vhashmap_iter_end(it.vhiter) || it.index >= it.vdict->values.metadata.length) {
		return true;
	} else {
		return false;
	}
}

bool vdict_iter_continue(VDictIter it) {
	if (vhashmap_iter_end(it.vhiter) || it.index >= it.vdict->values.metadata.length) {
		return false;
	} else {
		return true;
	}
}


size_t vdict_length(VDict *d) {
	return d->values.metadata.length;
}
