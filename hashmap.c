#include "hashmap.h"

bool hashmap_find(Hashmap *h, void *key, HashmapBucket **bucket, size_t *bindex, size_t *index) {
	HashPrecision hash;
	HashmapBucket *buck;
	uint8_t *data;
	size_t n;

	hash = hash_buf_length(key, h->klen);
	n = (size_t) (hash % ((HashPrecision) h->buckets.length));
	buck = array_get(&(h->buckets), n);

	if (bindex != NULL) {
		*bindex = n;
	}

	if (bucket != NULL) {
		*bucket = buck;
	}

	for (ArrayIter it = array_iter_first(buck); !array_iter_end(it); array_iter_next(&it)) {
		data = array_iter_get(it);

		if (memcmp(data, key, h->klen) == 0) {

			if (index != NULL) {
				*index = it.index;
			}

			return true;
		}
	}

	return false;
}

void hashmap_init(Hashmap *h, size_t klen, size_t vlen, size_t n) {
	HashmapBucket bucket;

	array_init(&(h->buckets), sizeof(HashmapBucket));
	h->klen = klen;
	h->vlen = vlen;

	for (size_t i = 0; i < n; ++i) {
		array_init(&bucket, klen + vlen);
		array_push(&(h->buckets), &bucket);
	}
}

void hashmap_free(Hashmap *h) {
	HashmapBucket bucket;

	for (ArrayIter it = array_iter_first(&(h->buckets)); !array_iter_end(it); array_iter_next(&it)) {
		array_iter_read(it, &bucket);
		array_free(&bucket);
	}

	array_free(&(h->buckets));

	h->klen = 0;
	h->vlen = 0;
}


void hashmap_resize(Hashmap *h, size_t n) {
	Hashmap t;

	hashmap_init(&t, h->klen, h->vlen, n);
	for (HashmapIter it = hashmap_iter_first(h); !hashmap_iter_end(it); hashmap_iter_next(&it)) {
		hashmap_set(&t, hashmap_iter_get_key(it), hashmap_iter_get(it));
	}

	hashmap_free(h);
	*h = t;
}


void hashmap_set(Hashmap *h, void *key, void *x) {
	HashmapBucket *bucket;
	uint8_t *data;
	size_t bindex;
	size_t index;

	if (hashmap_find(h, key, &bucket, &bindex, &index)) {
		// Set value only
		data = array_get(bucket, index);
		memmove(&(data[h->klen]), x, h->vlen);
	} else {
		// Insert data
		array_push(bucket, NULL);
		data = array_get(bucket, bucket->length-1);

		memmove(&(data[0]), key, h->klen);
		memmove(&(data[h->klen]), x, h->vlen);
	}
}

bool hashmap_del(Hashmap *h, void *key) {
	HashmapBucket *bucket;
	size_t bindex;
	size_t index;

	if (hashmap_find(h, key, &bucket, &bindex, &index)) {
		array_erase(bucket, index);
		return true;
	}

	return false;
}

bool hashmap_read(Hashmap *h, void *key, void *x) {
	HashmapBucket *bucket;
	uint8_t *data;
	size_t index;

	if (hashmap_find(h, key, &bucket, NULL, &index)) {
		data = array_get(bucket, index);

		if (x != NULL) {
			memmove(x, &(data[h->klen]), h->vlen);
		}

		return true;
	}

	return false;
}

void *hashmap_get(Hashmap *h, void *key) {
	HashmapBucket *bucket;
	uint8_t *data;
	size_t index;

	if (hashmap_find(h, key, &bucket, NULL, &index)) {
		data = array_get(bucket, index);
		return &(data[h->klen]);
	}

	return NULL;
}


HashmapIter hashmap_iter_first(Hashmap *h) {
	HashmapIter it;
	HashmapBucket *bucket;

	it.hashmap = h;
	it.bucket = HASHMAP_INVALID_INDEX;
	it.index = HASHMAP_INVALID_INDEX;

	for (ArrayIter jt = array_iter_first(&(h->buckets)); !array_iter_end(jt); array_iter_next(&jt)) {
		bucket = array_iter_get(jt);
		if (bucket->length > 0) {
			it.bucket = jt.index;
			it.index = 0;
			break;
		}
	}

	return it;
}

HashmapIter hashmap_iter_find(Hashmap *h, void *key) {
	HashmapIter it;

	it.hashmap = h;
	it.bucket = HASHMAP_INVALID_INDEX;
	it.index = HASHMAP_INVALID_INDEX;

	if (!hashmap_find(h, key, NULL, &(it.bucket), &(it.index))) {
		it.bucket = HASHMAP_INVALID_INDEX;
	}

	return it;
}

HashmapIter hashmap_iter_last(Hashmap *h) {
	HashmapIter it;
	HashmapBucket *bucket;

	it.hashmap = h;
	it.bucket = HASHMAP_INVALID_INDEX;
	it.index = HASHMAP_INVALID_INDEX;

	for (ArrayIter jt = array_iter_last(&(h->buckets)); !array_iter_end(jt); array_iter_prev(&jt)) {
		bucket = array_iter_get(jt);
		if (bucket->length > 0) {
			it.bucket = jt.index;
			it.index = bucket->length-1;
			break;
		}
	}

	return it;
}

HashmapIter hashmap_iter_insert(HashmapIter *it, void *key, void *x) {
	HashmapBucket *bucket;
	uint8_t *data;
	HashmapIter nt;
	size_t bindex;
	size_t index;

	nt.hashmap = it->hashmap;
	nt.bucket = it->bucket;
	nt.index = it->index;

	if (hashmap_find(it->hashmap, key, &bucket, &bindex, &index)) {
		// Set value only
		data = array_get(bucket, index);
		if (x != NULL)
			memmove(&(data[it->hashmap->klen]), x, it->hashmap->vlen);
	} else {
		// Insert data
		if (hashmap_iter_continue(*it) && it->bucket == bindex) {
			array_insert(bucket, it->index, NULL);
			data = array_get(bucket, it->index);

			if (key != NULL)
				memmove(&(data[0]), key, it->hashmap->klen);

			if (x != NULL)
				memmove(&(data[it->hashmap->klen]), x, it->hashmap->vlen);

			++(it->index);
		} else {
			array_push(bucket, NULL);
			data = array_get(bucket, bucket->length-1);

			if (key != NULL)
				memmove(&(data[0]), key, it->hashmap->klen);

			if (x != NULL)
				memmove(&(data[it->hashmap->klen]), x, it->hashmap->vlen);

			nt.bucket = bindex;
			nt.index = bucket->length-1;
		}
	}

	return nt;
}

// Must erase the curren item, and return the next iterator.
HashmapIter hashmap_iter_erase(HashmapIter *it) {
	HashmapBucket *bucket;
	HashmapIter nt;

	nt.hashmap = it->hashmap;
	nt.bucket = it->bucket;
	nt.index = it->index;

	if (hashmap_iter_continue(*it)) {
		hashmap_iter_next(&nt);
		bucket = array_get(&(it->hashmap->buckets), it->bucket);
		array_erase(bucket, it->index);

		if (hashmap_iter_end(nt)) {
			*it = hashmap_iter_last(it->hashmap);
		} else {
			it->bucket = nt.bucket;
			it->index = nt.index;
			hashmap_iter_prev(it);
		}
	}

	return nt;
}

void hashmap_iter_next(HashmapIter *it) {
	HashmapBucket *bucket;
	if (hashmap_iter_continue(*it)) {
		bucket = array_get(&(it->hashmap->buckets), it->bucket);

		++(it->index);

		if (it->index >= bucket->length) {
			for (ArrayIter jt = array_iter_find(&(it->hashmap->buckets), it->bucket+1); !array_iter_end(jt); array_iter_next(&jt)) {
				bucket = array_iter_get(jt);
				if (bucket->length > 0) {
					it->bucket = jt.index;
					it->index = 0;
					return;
				}
			}

			it->bucket = HASHMAP_INVALID_INDEX;
			it->index = HASHMAP_INVALID_INDEX;
		}
	}
}

void hashmap_iter_prev(HashmapIter *it) {
	HashmapBucket *bucket;
	if (hashmap_iter_continue(*it)) {
		bucket = array_get(&(it->hashmap->buckets), it->bucket);

		--(it->index);

		if (it->index >= bucket->length) {
			for (ArrayIter jt = array_iter_find(&(it->hashmap->buckets), it->bucket-1); !array_iter_end(jt); array_iter_prev(&jt)) {
				bucket = array_iter_get(jt);
				if (bucket->length > 0) {
					it->bucket = jt.index;
					it->index = bucket->length-1;
					return;
				}
			}

			it->bucket = HASHMAP_INVALID_INDEX;
			it->index = HASHMAP_INVALID_INDEX;
		}
	}
}


void hashmap_iter_set(HashmapIter it, void *x) {
	HashmapBucket *bucket;
	uint8_t *data;
	if (hashmap_iter_continue(it)) {
		bucket = array_get(&(it.hashmap->buckets), it.bucket);
		data = array_get(bucket, it.index);

		if (x != NULL)
			memmove(&(data[it.hashmap->klen]), x, it.hashmap->vlen);
	}
}

void hashmap_iter_read(HashmapIter it, void *x) {
	HashmapBucket *bucket;
	uint8_t *data;

	if (hashmap_iter_continue(it)) {
		bucket = array_get(&(it.hashmap->buckets), it.bucket);

		data = array_get(bucket, it.index);

		if (x != NULL) {
			memmove(x, &(data[it.hashmap->klen]), it.hashmap->vlen);
		}
	}
}

void *hashmap_iter_get(HashmapIter it) {
	HashmapBucket *bucket;
	uint8_t *data;

	if (hashmap_iter_continue(it)) {
		bucket = array_get(&(it.hashmap->buckets), it.bucket);
		data = array_get(bucket, it.index);
		return &(data[it.hashmap->klen]);
	} else {
		return NULL;
	}
}


void hashmap_iter_read_key(HashmapIter it, void *key) {
	HashmapBucket *bucket;
	uint8_t *data;

	if (hashmap_iter_continue(it)) {
		bucket = array_get(&(it.hashmap->buckets), it.bucket);

		data = array_get(bucket, it.index);

		if (key != NULL) {
			memmove(key, data, it.hashmap->klen);
		}
	}
}

void *hashmap_iter_get_key(HashmapIter it) {
	HashmapBucket *bucket;
	void *key;

	if (hashmap_iter_continue(it)) {
		bucket = array_get(&(it.hashmap->buckets), it.bucket);
		key = array_get(bucket, it.index);
		return key;
	} else {
		return NULL;
	}
}


bool hashmap_iter_end(HashmapIter it) {
	HashmapBucket *bucket;
	if (it.bucket < it.hashmap->buckets.length) {
		bucket = array_get(&(it.hashmap->buckets), it.bucket);
		if (it.index < bucket->length) {
			return false;
		} else {
			return true;
		}
	} else {
		return true;
	}
}

bool hashmap_iter_continue(HashmapIter it) {
	HashmapBucket *bucket;
	if (it.bucket < it.hashmap->buckets.length) {
		bucket = array_get(&(it.hashmap->buckets), it.bucket);
		if (it.index < bucket->length) {
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}
