#include "cache.h"

void cache_init(Cache *c, size_t klen, size_t vlen, size_t n) {
	array_init(&(c->values), vlen);
	hashmap_init(&(c->indices), klen, sizeof(size_t), n);
}

void cache_free(Cache *c) {
	hashmap_free(&(c->indices));
	array_free(&(c->values));
}


void cache_resize(Cache *c, size_t n) {
	hashmap_resize(&(c->indices), n);
}


void cache_set(Cache *c, void *key, void *x) {
	HashmapBucket *bucket;
	uint8_t *data;
	size_t bindex;
	size_t index;
	size_t vindex;

	if (hashmap_find(&(c->indices), key, &bucket, &bindex, &index)) {
		data = array_get(bucket, index);
		memmove(&vindex, &(data[c->indices.klen]), sizeof(size_t));
		array_set(&(c->values), vindex, x);
	} else {
		vindex = c->values.length;
		array_push(&(c->values), x);
		hashmap_set(&(c->indices), key, &vindex);
	}
}

bool cache_del(Cache *c, void *key) {
	HashmapBucket *bucket;
	uint8_t *data;
	size_t bindex;
	size_t index;
	size_t vindex;

	if (hashmap_find(&(c->indices), key, &bucket, &bindex, &index)) {
		data = array_get(bucket, index);
		memmove(&vindex, &(data[c->indices.klen]), sizeof(size_t));

		array_erase(bucket, index);
		array_erase(&(c->values), vindex);

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

bool cache_read(Cache *c, void *key, void *x) {
	HashmapBucket *bucket;
	uint8_t *data;
	size_t bindex;
	size_t index;
	size_t vindex;

	if (hashmap_find(&(c->indices), key, &bucket, &bindex, &index)) {
		data = array_get(bucket, index);
		memmove(&vindex, &(data[c->indices.klen]), sizeof(size_t));
		array_read(&(c->values), vindex, x);
		return true;
	} else {
		return false;
	}
}

void *cache_get(Cache *c, void *key) {
	HashmapBucket *bucket;
	uint8_t *data;
	size_t bindex;
	size_t index;
	size_t vindex;

	if (hashmap_find(&(c->indices), key, &bucket, &bindex, &index)) {
		data = array_get(bucket, index);
		memmove(&vindex, &(data[c->indices.klen]), sizeof(size_t));
		return array_get(&(c->values), vindex);
	} else {
		return NULL;
	}
}


CacheIter cache_iter_first(Cache *c) {
	CacheIter it;

	it.cache = c;
	it.hiter = hashmap_iter_first(&(c->indices));
	hashmap_iter_read(it.hiter, &(it.index));

	return it;
}

CacheIter cache_iter_find(Cache *c, void *key) {
	CacheIter it;

	it.cache = c;
	it.hiter = hashmap_iter_find(&(c->indices), key);
	hashmap_iter_read(it.hiter, &(it.index));

	return it;
}

CacheIter cache_iter_last(Cache *c) {
	CacheIter it;

	it.cache = c;
	it.hiter = hashmap_iter_last(&(c->indices));
	hashmap_iter_read(it.hiter, &(it.index));

	return it;
}


CacheIter cache_iter_insert(CacheIter *it, void *key, void *x) {
	CacheIter nt;

	nt.cache = it->cache;
	nt.hiter = hashmap_iter_insert(&(it->hiter), key, NULL);
	nt.index = CACHE_INVALID_INDEX;

	// If the element was inserted, set index and x.
	if (hashmap_iter_continue(nt.hiter)) {
		if (nt.hiter.bucket != it->hiter.bucket || nt.hiter.index != it->hiter.index) {
			nt.index = it->cache->values.length;
			array_push(&(it->cache->values), x);
			memmove(hashmap_iter_get(nt.hiter), &(nt.index), sizeof(size_t));
		}
	}

	hashmap_iter_read(nt.hiter, &(nt.index));
	hashmap_iter_read(it->hiter, &(it->index));

	return nt;
}

CacheIter cache_iter_erase(CacheIter *it) {
	CacheIter nt;
	size_t index;

	nt.cache = it->cache;
	nt.index = CACHE_INVALID_INDEX;

	if (cache_iter_continue(*it)) {
		array_erase(&(it->cache->values), it->index);
		nt.hiter = hashmap_iter_erase(&(it->hiter));

		// When an vindex is removed, must update all others
		for (HashmapIter ot = hashmap_iter_first(&(it->cache->indices)); !hashmap_iter_end(ot); hashmap_iter_next(&ot)) {
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

void cache_iter_next(CacheIter *it) {
	hashmap_iter_next(&(it->hiter));
	if (hashmap_iter_continue(it->hiter)) {
		hashmap_iter_read(it->hiter, &(it->index));
	} else {
		it->index = CACHE_INVALID_INDEX;
	}
}

void cache_iter_prev(CacheIter *it) {
	hashmap_iter_prev(&(it->hiter));
	if (hashmap_iter_continue(it->hiter)) {
		hashmap_iter_read(it->hiter, &(it->index));
	} else {
		it->index = CACHE_INVALID_INDEX;
	}
}


void cache_iter_set(CacheIter it, void *x) {
	if (cache_iter_continue(it)) {
		array_set(&(it.cache->values), it.index, x);
	}
}

void cache_iter_read(CacheIter it, void *x) {
	if (cache_iter_continue(it)) {
		array_read(&(it.cache->values), it.index, x);
	}
}

void *cache_iter_get(CacheIter it) {
	if (cache_iter_continue(it)) {
		return array_get(&(it.cache->values), it.index);
	} else {
		return NULL;
	}
}


void cache_iter_read_key(CacheIter it, void *key) {
	hashmap_iter_read_key(it.hiter, key);
}

void *cache_iter_get_key(CacheIter it) {
	return hashmap_iter_get_key(it.hiter);
}


bool cache_iter_end(CacheIter it) {
	if (hashmap_iter_end(it.hiter) || it.index >= it.cache->values.length) {
		return true;
	} else {
		return false;
	}
}

bool cache_iter_continue(CacheIter it) {
	if (hashmap_iter_end(it.hiter) || it.index >= it.cache->values.length) {
		return false;
	} else {
		return true;
	}
}

size_t cache_length(Cache *c) {
	return c->values.length;
}
