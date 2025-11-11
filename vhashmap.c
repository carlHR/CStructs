#include "vhashmap.h"

bool vhashmap_find(VHashmap *h, void *key, size_t klen, VHashmapBucket **bucket, size_t *bindex, size_t *index) {
	HashPrecision hash;
	VHashmapBucket *buck;
	uint8_t *data;
	size_t dlen;
	size_t n;

	hash = hash_buf_length(key, klen);
	n = (size_t) (hash % ((HashPrecision) h->buckets.length));
	buck = array_get(&(h->buckets), n);

	if (bindex != NULL) {
		*bindex = n;
	}

	if (bucket != NULL) {
		*bucket = buck;
	}

	for (VArrayIter it = varray_iter_first(buck); !varray_iter_end(it); varray_iter_next(&it)) {
		data = varray_iter_get(it, &dlen);

		if (((dlen - (h->vlen)) == klen) && (memcmp(data, key, klen) == 0)) {

			if (index != NULL) {
				*index = it.mditer.index;
			}

			return true;
		}
	}

	return false;
}


void vhashmap_init(VHashmap *h, size_t vlen, size_t n) {
	VHashmapBucket bucket;

	array_init(&(h->buckets), sizeof(VHashmapBucket));
	h->vlen = vlen;

	for (size_t i = 0; i < n; ++i) {
		varray_init(&bucket);
		array_push(&(h->buckets), &bucket);
	}
}

void vhashmap_free(VHashmap *h) {
	VHashmapBucket bucket;

	for (ArrayIter it = array_iter_first(&(h->buckets)); !array_iter_end(it); array_iter_next(&it)) {
		array_iter_read(it, &bucket);
		varray_free(&bucket);
	}

	array_free(&(h->buckets));

	h->vlen = 0;
}


void vhashmap_resize(VHashmap *h, size_t n) {
	VHashmap t;
	void *key;
	void *val;
	size_t klen;

	vhashmap_init(&t, h->vlen, n);
	for (VHashmapIter it = vhashmap_iter_first(h); !vhashmap_iter_end(it); vhashmap_iter_next(&it)) {
		key = vhashmap_iter_get_key(it, &klen);
		val = vhashmap_iter_get(it);
		vhashmap_set(&t, key, klen, val);
	}

	vhashmap_free(h);
	*h = t;
}

void vhashmap_set(VHashmap *h, void *key, size_t klen, void *x) {
	VHashmapBucket *bucket;
	uint8_t *data;
	size_t bindex;
	size_t index;

	if (vhashmap_find(h, key, klen, &bucket, &bindex, &index)) {
		// klen won't change, otherwise its an insert operation.
		// Set value only
		data = varray_get(bucket, index, NULL);

		if (x != NULL)
			memmove(&(data[klen]), x, h->vlen);
	} else {
		// Insert data
		varray_push(bucket, NULL, (klen + (h->vlen)));
		data = varray_get(bucket, bucket->metadata.length-1, NULL);

		if (key != NULL)
			memmove(&(data[0]), key, klen);

		if (x != NULL)
			memmove(&(data[klen]), x, h->vlen);
	}
}

bool vhashmap_del(VHashmap *h, void *key, size_t klen) {
	VHashmapBucket *bucket;
	size_t bindex;
	size_t index;

	if (vhashmap_find(h, key, klen, &bucket, &bindex, &index)) {
		varray_erase(bucket, index);
		return true;
	}

	return false;
}

bool vhashmap_read(VHashmap *h, void *key, size_t klen, void *x) {
	VHashmapBucket *bucket;
	uint8_t *data;
	size_t index;

	if (vhashmap_find(h, key, klen, &bucket, NULL, &index)) {
		data = varray_get(bucket, index, NULL);

		if (x != NULL) {
			memmove(x, &(data[klen]), h->vlen);
		}

		return true;
	}

	return false;
}

void *vhashmap_get(VHashmap *h, void *key, size_t klen) {
	VHashmapBucket *bucket;
	uint8_t *data;
	size_t index;

	if (vhashmap_find(h, key, klen, &bucket, NULL, &index)) {
		data = varray_get(bucket, index, NULL);
		return &(data[klen]);
	}

	return NULL;
}


VHashmapIter vhashmap_iter_first(VHashmap *h) {
	VHashmapIter it;
	VHashmapBucket *bucket;

	it.vhashmap = h;
	it.bucket = VHASHMAP_INVALID_INDEX;
	it.index = VHASHMAP_INVALID_INDEX;

	for (ArrayIter jt = array_iter_first(&(h->buckets)); !array_iter_end(jt); array_iter_next(&jt)) {
		bucket = array_iter_get(jt);
		if (bucket->metadata.length > 0) {
			it.bucket = jt.index;
			it.index = 0;
			break;
		}
	}

	return it;
}

VHashmapIter vhashmap_iter_find(VHashmap *h, void *key, size_t klen) {
	VHashmapIter it;

	it.vhashmap = h;
	it.bucket = VHASHMAP_INVALID_INDEX;
	it.index = VHASHMAP_INVALID_INDEX;

	if (!vhashmap_find(h, key, klen, NULL, &(it.bucket), &(it.index))) {
		it.bucket = VHASHMAP_INVALID_INDEX;
	}

	return it;
}

VHashmapIter vhashmap_iter_last(VHashmap *h) {
	VHashmapIter it;
	VHashmapBucket *bucket;

	it.vhashmap = h;
	it.bucket = VHASHMAP_INVALID_INDEX;
	it.index = VHASHMAP_INVALID_INDEX;

	for (ArrayIter jt = array_iter_last(&(h->buckets)); !array_iter_end(jt); array_iter_prev(&jt)) {
		bucket = array_iter_get(jt);
		if (bucket->metadata.length > 0) {
			it.bucket = jt.index;
			it.index = bucket->metadata.length-1;
			break;
		}
	}

	return it;
}


VHashmapIter vhashmap_iter_insert(VHashmapIter *it, void *key, size_t klen, void *x) {
	VHashmapBucket *bucket;
	uint8_t *data;
	VHashmapIter nt;
	size_t bindex;
	size_t index;

	nt.vhashmap = it->vhashmap;
	nt.bucket = it->bucket;
	nt.index = it->index;

	if (vhashmap_find(it->vhashmap, key, klen, &bucket, &bindex, &index)) {
		// Set value only
		data = varray_get(bucket, index, NULL);

		if (x != NULL)
			memmove(&(data[klen]), x, it->vhashmap->vlen);

	} else {
		// Insert data
		if (vhashmap_iter_continue(*it) && it->bucket == bindex) {
			varray_insert(bucket, it->index, NULL, (klen + (it->vhashmap->vlen)));
			data = varray_get(bucket, it->index, NULL);

			if (key != NULL)
				memmove(&(data[0]), key, klen);

			if (x != NULL)
				memmove(&(data[klen]), x, it->vhashmap->vlen);

			++(it->index);
		} else {
			varray_push(bucket, NULL, (klen + (it->vhashmap->vlen)));
			data = varray_get(bucket, bucket->metadata.length-1, NULL);

			if (key != NULL)
				memmove(&(data[0]), key, klen);

			if (x != NULL)
				memmove(&(data[klen]), x, it->vhashmap->vlen);

			nt.bucket = bindex;
			nt.index = bucket->metadata.length-1;
		}
	}

	return nt;
}

VHashmapIter vhashmap_iter_erase(VHashmapIter *it) {
	VHashmapBucket *bucket;
	VHashmapIter nt;

	nt.vhashmap = it->vhashmap;
	nt.bucket = it->bucket;
	nt.index = it->index;

	if (vhashmap_iter_continue(*it)) {
		vhashmap_iter_next(&nt);
		bucket = array_get(&(it->vhashmap->buckets), it->bucket);
		varray_erase(bucket, it->index);

		if (vhashmap_iter_end(nt)) {
			*it = vhashmap_iter_last(it->vhashmap);
		} else {
			it->bucket = nt.bucket;
			it->index = nt.index;
			vhashmap_iter_prev(it);
		}
	}

	return nt;
}

void vhashmap_iter_next(VHashmapIter *it) {
	VHashmapBucket *bucket;
	if (vhashmap_iter_continue(*it)) {
		bucket = array_get(&(it->vhashmap->buckets), it->bucket);

		++(it->index);

		if (it->index >= bucket->metadata.length) {
			for (ArrayIter jt = array_iter_find(&(it->vhashmap->buckets), it->bucket+1); !array_iter_end(jt); array_iter_next(&jt)) {
				bucket = array_iter_get(jt);
				if (bucket->metadata.length > 0) {
					it->bucket = jt.index;
					it->index = 0;
					return;
				}
			}

			it->bucket = VHASHMAP_INVALID_INDEX;
			it->index = VHASHMAP_INVALID_INDEX;
		}
	}
}

void vhashmap_iter_prev(VHashmapIter *it) {
	VHashmapBucket *bucket;
	if (vhashmap_iter_continue(*it)) {
		bucket = array_get(&(it->vhashmap->buckets), it->bucket);

		--(it->index);

		if (it->index >= bucket->metadata.length) {
			for (ArrayIter jt = array_iter_find(&(it->vhashmap->buckets), it->bucket-1); !array_iter_end(jt); array_iter_prev(&jt)) {
				bucket = array_iter_get(jt);
				if (bucket->metadata.length > 0) {
					it->bucket = jt.index;
					it->index = bucket->metadata.length-1;
					return;
				}
			}

			it->bucket = VHASHMAP_INVALID_INDEX;
			it->index = VHASHMAP_INVALID_INDEX;
		}
	}
}


void vhashmap_iter_set(VHashmapIter it, void *x) {
	VHashmapBucket *bucket;
	uint8_t *data;
	size_t dlen;
	if (vhashmap_iter_continue(it)) {
		bucket = array_get(&(it.vhashmap->buckets), it.bucket);
		data = varray_get(bucket, it.index, &dlen);

		if (x != NULL)
			memmove(&(data[dlen - (it.vhashmap->vlen)]), x, it.vhashmap->vlen);
	}
}

void vhashmap_iter_read(VHashmapIter it, void *x) {
	VHashmapBucket *bucket;
	uint8_t *data;
	size_t dlen;

	if (vhashmap_iter_continue(it)) {
		bucket = array_get(&(it.vhashmap->buckets), it.bucket);

		data = varray_get(bucket, it.index, &dlen);

		if (x != NULL) {
			memmove(x, &(data[dlen - (it.vhashmap->vlen)]), it.vhashmap->vlen);
		}
	}
}

void *vhashmap_iter_get(VHashmapIter it) {
	VHashmapBucket *bucket;
	uint8_t *data;
	size_t dlen;

	if (vhashmap_iter_continue(it)) {
		bucket = array_get(&(it.vhashmap->buckets), it.bucket);
		data = varray_get(bucket, it.index, &dlen);
		return &(data[dlen - (it.vhashmap->vlen)]);
	} else {
		return NULL;
	}
}


void vhashmap_iter_read_key(VHashmapIter it, void *key, size_t *klen) {
	VHashmapBucket *bucket;
	uint8_t *data;
	size_t dlen;

	if (vhashmap_iter_continue(it)) {
		bucket = array_get(&(it.vhashmap->buckets), it.bucket);

		data = varray_get(bucket, it.index, &dlen);

		if (klen != NULL) {
			*klen = (dlen - (it.vhashmap->vlen));
		}

		if (key != NULL) {
			memmove(key, data, (dlen - (it.vhashmap->vlen)));
		}
	}
}

void *vhashmap_iter_get_key(VHashmapIter it, size_t *klen) {
	VHashmapBucket *bucket;
	void *key;
	size_t dlen;

	if (vhashmap_iter_continue(it)) {
		bucket = array_get(&(it.vhashmap->buckets), it.bucket);
		key = varray_get(bucket, it.index, &dlen);

		if (klen != NULL) {
			*klen = (dlen - (it.vhashmap->vlen));
		}

		return key;
	} else {
		return NULL;
	}
}


bool vhashmap_iter_end(VHashmapIter it) {
	VHashmapBucket *bucket;
	if (it.bucket < it.vhashmap->buckets.length) {
		bucket = array_get(&(it.vhashmap->buckets), it.bucket);
		if (it.index < bucket->metadata.length) {
			return false;
		} else {
			return true;
		}
	} else {
		return true;
	}
}

bool vhashmap_iter_continue(VHashmapIter it) {
	VHashmapBucket *bucket;
	if (it.bucket < it.vhashmap->buckets.length) {
		bucket = array_get(&(it.vhashmap->buckets), it.bucket);
		if (it.index < bucket->metadata.length) {
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}
