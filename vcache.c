#include "vcache.h"

void vcache_init(VCache *c, size_t klen, size_t n) {
	varray_init(&(c->values));
	hashmap_init(&(c->indices), klen, sizeof(size_t), n);
}

void vcache_free(VCache *c) {
	hashmap_free(&(c->indices));
	varray_free(&(c->values));
}


void vcache_resize(VCache *c, size_t n) {
	hashmap_resize(&(c->indices), n);
}


void vcache_set(VCache *c, void *key, void *x, size_t vlen) {
	HashmapBucket *bucket;
	uint8_t *data;
	size_t bindex;
	size_t index;
	size_t vindex;

	if (hashmap_find(&(c->indices), key, &bucket, &bindex, &index)) {
		data = array_get(bucket, index);
		memmove(&vindex, &(data[c->indices.klen]), sizeof(size_t));
		varray_set(&(c->values), vindex, x, vlen);
	} else {
		vindex = c->values.metadata.length;
		varray_push(&(c->values), x, vlen);
		hashmap_set(&(c->indices), key, &vindex);
	}
}

bool vcache_del(VCache *c, void *key) {
	HashmapBucket *bucket;
	uint8_t *data;
	size_t bindex;
	size_t index;
	size_t vindex;

	if (hashmap_find(&(c->indices), key, &bucket, &bindex, &index)) {
		data = array_get(bucket, index);
		memmove(&vindex, &(data[c->indices.klen]), sizeof(size_t));

		array_erase(bucket, index);
		varray_erase(&(c->values), vindex);

		// When an vindex is removed, must update all others
		for (HashmapIter it = hashmap_iter_first(&(c->indices)); !hashmap_iter_end(it); hashmap_iter_next(&it)) {
			hashmap_iter_read(it, &index);
			if (vindex < index) {
				--index;
				hashmap_iter_set(it, &index);
			}
		}

		return true;
	}

	return false;
}

bool vcache_read(VCache *c, void *key, void *x, size_t *vlen) {
	HashmapBucket *bucket;
	uint8_t *data;
	size_t bindex;
	size_t index;
	size_t vindex;

	if (hashmap_find(&(c->indices), key, &bucket, &bindex, &index)) {
		data = array_get(bucket, index);
		memmove(&vindex, &(data[c->indices.klen]), sizeof(size_t));
		varray_read(&(c->values), vindex, x, vlen);
		return true;
	} else {
		return false;
	}
}

void *vcache_get(VCache *c, void *key, size_t *vlen) {
	HashmapBucket *bucket;
	uint8_t *data;
	size_t bindex;
	size_t index;
	size_t vindex;

	if (hashmap_find(&(c->indices), key, &bucket, &bindex, &index)) {
		data = array_get(bucket, index);
		memmove(&vindex, &(data[c->indices.klen]), sizeof(size_t));
		return varray_get(&(c->values), vindex, vlen);
	} else {
		return NULL;
	}
}


VCacheIter vcache_iter_first(VCache *c) {
	VCacheIter it;

	it.vcache = c;
	it.hiter = hashmap_iter_first(&(c->indices));
	hashmap_iter_read(it.hiter, &(it.index));

	return it;
}

VCacheIter vcache_iter_find(VCache *c, void *key) {
	VCacheIter it;

	it.vcache = c;
	it.hiter = hashmap_iter_find(&(c->indices), key);
	hashmap_iter_read(it.hiter, &(it.index));

	return it;
}

VCacheIter vcache_iter_last(VCache *c) {
	VCacheIter it;

	it.vcache = c;
	it.hiter = hashmap_iter_last(&(c->indices));
	hashmap_iter_read(it.hiter, &(it.index));

	return it;
}


VCacheIter vcache_iter_insert(VCacheIter *it, void *key, void *x, size_t vlen) {
	VCacheIter nt;

	nt.vcache = it->vcache;
	nt.hiter = hashmap_iter_insert(&(it->hiter), key, NULL);
	nt.index = VCACHE_INVALID_INDEX;

	// If the element was inserted, set index and x.
	if (hashmap_iter_continue(nt.hiter)) {
		if (nt.hiter.bucket != it->hiter.bucket || nt.hiter.index != it->hiter.index) {
			nt.index = it->vcache->values.metadata.length;
			varray_push(&(it->vcache->values), x, vlen);
			memmove(hashmap_iter_get(nt.hiter), &(nt.index), sizeof(size_t));
		}
	}

	hashmap_iter_read(nt.hiter, &(nt.index));
	hashmap_iter_read(it->hiter, &(it->index));

	return nt;
}

VCacheIter vcache_iter_erase(VCacheIter *it) {
	VCacheIter nt;
	size_t index;

	nt.vcache = it->vcache;
	nt.index = VCACHE_INVALID_INDEX;

	if (vcache_iter_continue(*it)) {
		varray_erase(&(it->vcache->values), it->index);
		nt.hiter = hashmap_iter_erase(&(it->hiter));

		// When an vindex is removed, must update all others
		for (HashmapIter ot = hashmap_iter_first(&(it->vcache->indices)); !hashmap_iter_end(ot); hashmap_iter_next(&ot)) {
			hashmap_iter_read(ot, &index);
			if (it->index < index) {
				--index;
				hashmap_iter_set(ot, &index);
			}
		}

		hashmap_iter_read(nt.hiter, &(nt.index));
		hashmap_iter_read(it->hiter, &(it->index));
	}

	return nt;
}

void vcache_iter_next(VCacheIter *it) {
	hashmap_iter_next(&(it->hiter));
	if (hashmap_iter_continue(it->hiter)) {
		hashmap_iter_read(it->hiter, &(it->index));
	} else {
		it->index = VCACHE_INVALID_INDEX;
	}
}

void vcache_iter_prev(VCacheIter *it) {
	hashmap_iter_prev(&(it->hiter));
	if (hashmap_iter_continue(it->hiter)) {
		hashmap_iter_read(it->hiter, &(it->index));
	} else {
		it->index = VCACHE_INVALID_INDEX;
	}
}


void vcache_iter_set(VCacheIter it, void *x, size_t vlen) {
	if (vcache_iter_continue(it)) {
		varray_set(&(it.vcache->values), it.index, x, vlen);
	}
}

void vcache_iter_read(VCacheIter it, void *x, size_t *vlen) {
	if (vcache_iter_continue(it)) {
		varray_read(&(it.vcache->values), it.index, x, vlen);
	}
}

void *vcache_iter_get(VCacheIter it, size_t *vlen) {
	if (vcache_iter_continue(it)) {
		return varray_get(&(it.vcache->values), it.index, vlen);
	} else {
		return NULL;
	}
}


void vcache_iter_read_key(VCacheIter it, void *key) {
	hashmap_iter_read_key(it.hiter, key);
}

void *vcache_iter_get_key(VCacheIter it) {
	return hashmap_iter_get_key(it.hiter);
}


bool vcache_iter_end(VCacheIter it) {
	if (hashmap_iter_end(it.hiter) || it.index >= it.vcache->values.metadata.length) {
		return true;
	} else {
		return false;
	}
}

bool vcache_iter_continue(VCacheIter it) {
	if (hashmap_iter_end(it.hiter) || it.index >= it.vcache->values.metadata.length) {
		return false;
	} else {
		return true;
	}
}


size_t vcache_length(VCache *c) {
	return c->values.metadata.length;
}

