#include "varray.h"

void varray_init(VArray *v) {
	array_init(&(v->data), 1);
	array_init(&(v->metadata), sizeof(VArrayMetadata));
}

void varray_free(VArray *v) {
	array_free(&(v->metadata));
	array_free(&(v->data));
}


void varray_reserve(VArray *v, size_t nitems, size_t datalen) {
	array_reserve(&(v->data), datalen);
	array_reserve(&(v->metadata), nitems);
}

void varray_shrink(VArray *v, size_t nitems, size_t datalen) {
	array_shrink(&(v->data), datalen);
	array_shrink(&(v->metadata), nitems);
}

void varray_fit(VArray *v, size_t nitems, size_t datalen) {
	array_fit(&(v->data));
	array_fit(&(v->metadata));
}

void varray_push(VArray *v, void *x, size_t len) {
	VArrayMetadata md;

	md.offset = v->data.length;
	md.length = len;

	array_reserve(&(v->data), v->data.length + len);

	if (x != NULL)
		memmove(&(v->data.data[v->data.length]), x, len);
	else
		memset(&(v->data.data[v->data.length]), '\0', len);

	v->data.data[md.offset + md.length] = '\0';

	array_push(&(v->metadata), &md);

	v->data.length += len;
}

void varray_pop(VArray *v) {
	VArrayMetadata md;

	array_read(&(v->metadata), v->metadata.length-1, &md);
	array_move(&(v->data), md.offset, ARRAY_MOVE_DEL, md.length, NULL);
	array_pop(&(v->metadata));
}

void varray_insert(VArray *v, size_t i, void *x, size_t len) {
	VArrayMetadata md;

	if (i > v->metadata.length) {
		return;
	} else if (i == v->metadata.length) {
		varray_push(v, x, len);
		return;
	}

	// Insert and erase are more complex than push and pop, as we need to synchronize
	// data and metadata. As we store the offset in metadata, we need to increase further
	// offsets if we insert data in the middle/beggining of the array.
	array_read(&(v->metadata), i, &md);

	array_move(&(v->data), md.offset, ARRAY_MOVE_ADD, len, NULL);

	if (x != NULL)
		memmove(&(v->data.data[md.offset]), x, len);
	else
		memset(&(v->data.data[md.offset]), '\0', len);
	
	md.length = len;
	v->data.data[md.offset + md.length] = '\0';
	
	array_insert(&(v->metadata), i, &md);

	for (ArrayIter it = array_iter_find(&(v->metadata), i+1); !array_iter_end(it); array_iter_next(&it)) {
		array_iter_read(it, &md);
		md.offset += len;
		array_iter_set(it, &md);
	}
}

void varray_erase(VArray *v, size_t i) {
	VArrayMetadata md;
	size_t len;

	if (i > v->metadata.length) {
		return;
	} else if (i == v->metadata.length) {
		varray_pop(v);
		return;
	}

	// Insert and erase are more complex than push and pop, as we need to synchronize
	// data and metadata. As we store the offset in metadata, we need to decrease further
	// offsets if we erase data in the middle/beggining of the array.
	array_read(&(v->metadata), i, &md);
	len = md.length;

	array_move(&(v->data), md.offset, ARRAY_MOVE_DEL, md.length, NULL);
	array_erase(&(v->metadata), i);

	for (ArrayIter it = array_iter_find(&(v->metadata), i); !array_iter_end(it); array_iter_next(&it)) {
		array_iter_read(it, &md);
		md.offset -= len;
		array_iter_set(it, &md);
	}
}

void varray_read(VArray *v, size_t i, void *x, size_t *len) {
	VArrayMetadata md;

	if (i >= v->metadata.length) {
		return;
	}

	array_read(&(v->metadata), i, &md);
	if (len != NULL)
		*len = md.length;

	if (x != NULL)
		memmove(x, &(v->data.data[md.offset]), md.length);
}

void varray_set(VArray *v, size_t i, void *x, size_t newlen) {
	VArrayMetadata md;
	size_t diff;

	if (newlen == 0) {
		varray_erase(v, i);
		return;
	}

	if (i >= v->metadata.length) {
		return;
	}

	// Set, as with insert/erase, also potentially changes the offset of further
	// items on the varray. For this reason, we need to iterate through other
	// elements and tell their new offsets.
	//
	// If newlen is zero, this function behaves the same as varray_erase.
	array_read(&(v->metadata), i, &md);

	// No offset change. Faster.
	if (md.length == newlen) {

		if (x != NULL)
			memmove(&(v->data.data[md.offset]), x, newlen);
		else
			memset(&(v->data.data[md.offset]), '\0', newlen);

	// Changes the offset. Needs to iterate.
	} else {
		// Insert data
		if (newlen > md.length) {
			diff = newlen - md.length;

			array_move(&(v->data), md.offset, ARRAY_MOVE_ADD, diff, NULL);

			if (x != NULL)
				memmove(&(v->data.data[md.offset]), x, newlen);
			else
				memset(&(v->data.data[md.offset]), '\0', newlen);

			md.length = newlen;
			array_set(&(v->metadata), i, &md);

			for (ArrayIter it = array_iter_find(&(v->metadata), i+1); !array_iter_end(it); array_iter_next(&it)) {
				array_iter_read(it, &md);
				md.offset += diff;
				array_iter_set(it, &md);
			}

		// Erase data
		} else {
			diff = md.length - newlen;

			array_move(&(v->data), md.offset, ARRAY_MOVE_DEL, diff, NULL);

			if (x != NULL)
				memmove(&(v->data.data[md.offset]), x, newlen);
			else
				memset(&(v->data.data[md.offset]), '\0', newlen);

			md.length = newlen;
			array_set(&(v->metadata), i, &md);

			for (ArrayIter it = array_iter_find(&(v->metadata), i+1); !array_iter_end(it); array_iter_next(&it)) {
				array_iter_read(it, &md);
				md.offset -= diff;
				array_iter_set(it, &md);
			}

		}
	}
}

void *varray_get(VArray *v, size_t i, size_t *len) {
	VArrayMetadata md;

	if (i >= v->metadata.length) {
		return NULL;
	}

	array_read(&(v->metadata), i, &md);

	if (len != NULL)
		*len = md.length;

	return &(v->data.data[md.offset]);
}


VArrayIter varray_iter_first(VArray *v) {
	VArrayIter it;

	it.varray = v;
	it.mditer = array_iter_first(&(v->metadata));

	return it;
}

VArrayIter varray_iter_last(VArray *v) {
	VArrayIter it;

	it.varray = v;
	it.mditer = array_iter_last(&(v->metadata));

	return it;
}

VArrayIter varray_iter_find(VArray *v, size_t index) {
	VArrayIter it;

	it.varray = v;
	it.mditer = array_iter_find(&(v->metadata), index);

	return it;
}


bool varray_iter_end(VArrayIter it) {
	return array_iter_end(it.mditer);
}

bool varray_iter_continue(VArrayIter it) {
	return array_iter_continue(it.mditer);
}


void varray_iter_next(VArrayIter *it) {
	array_iter_next(&(it->mditer));
}

void varray_iter_prev(VArrayIter *it) {
	array_iter_prev(&(it->mditer));
}


VArrayIter varray_iter_insert(VArrayIter *it, void *x, size_t len) {
	VArrayIter nt;
	nt.varray = it->varray;
	nt.mditer = it->mditer;
	if (varray_iter_continue(*it)) {
		varray_insert(it->varray, it->mditer.index, x, len);
		++(it->mditer.index);
	} else {
		varray_push(it->varray, x, len);
		nt.mditer.index = it->varray->metadata.length-1;
	}
	return nt;
}

VArrayIter varray_iter_erase(VArrayIter *it) {
	VArrayIter nt;
	nt.varray = it->varray;
	nt.mditer = it->mditer;
	if (varray_iter_continue(*it)) {
		varray_erase(it->varray, it->mditer.index);
		--nt.mditer.index;
	}
	return nt;
}

void varray_iter_set(VArrayIter it, void *x, size_t newlen) {
	varray_set(it.varray, it.mditer.index, x, newlen);
}

void varray_iter_read(VArrayIter it, void *x, size_t *len) {
	varray_read(it.varray, it.mditer.index, x, len);
}

void *varray_iter_get(VArrayIter it, size_t *len) {
	return varray_get(it.varray, it.mditer.index, len);
}
