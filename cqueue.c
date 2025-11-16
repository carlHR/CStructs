#include "cqueue.h"

static void cqueue_grow(CQueue *q) {
	size_t n;
	n = q->length + ((size_t)(((double) q->length) * q->policy.growth));
	if (n == q->length) {
		++n;
	}
	cqueue_resize(q, n);
}

static void cqueue_shrink(CQueue *q) {
	size_t n;
	if (q->count <= ((size_t) (((double) q->length) * q->policy.shrink_threshold))) {
		n = q->length - ((size_t)(((double) q->length) * q->policy.shrink));
		if (n < q->policy.minlen) {
			n = q->policy.minlen;
		}

		if (n < q->count) {
			n = q->count;
		}
		cqueue_resize(q, n);
	}
}

void cqueue_init(CQueue *q, size_t chunk, size_t minlen) {
	q->data = NULL;
	q->length = 0;
	q->count = 0;
	q->chunk = chunk;
	q->head = 0;

	q->policy.growth = CQUEUE_GROWTH;
	q->policy.shrink = CQUEUE_SHRINK;
	q->policy.shrink_threshold = CQUEUE_SHRINK_THRESHOLD;
	q->policy.minlen = minlen;

	if (minlen != 0) {
		q->data = memory_malloc(chunk * minlen);
		q->length = minlen;
	}
}

void cqueue_free(CQueue *q) {
	memory_free(q->data);
	q->data = NULL;
	q->length = 0;
	q->count = 0;
	q->head = 0;
	q->policy.minlen = 0;
}

// Possible cases:
// [_ _ _ _ _ _ _ _ _ _] H
// [H . . . . _ _ _ _ _]
// [. . . _ _ _ H . . .]
//
// Converts 2 -> 1
void cqueue_resize(CQueue *q, size_t n) {
	uint8_t *tmp;
	size_t m;

	if (n == q->length) {
		return;
	}

	tmp = memory_malloc((q->chunk) * n);

	if (q->count > 0 && q->data != NULL) {
		if (q->head + q->count <= q->length) {
			// Case 1
			m = q->count;
			memmove(tmp, &(q->data[(q->head) * (q->chunk)]), (m * (q->chunk)));
		} else {
			// Case 2
			m = (q->length) - (q->head) + 1;
			memmove(tmp, &(q->data[(q->head) * (q->chunk)]), m * (q->chunk));
			memmove(&(tmp[m * (q->chunk)]), &(q->data[0]), (q->count - m) * (q->chunk));
		}
	}

	memory_free(q->data);
	q->data = tmp;
	q->length = n;
	q->head = 0;
}

void cqueue_push(CQueue *q, void *x) {
	size_t n;

	if (q->count + 1 >= q->length) {
		cqueue_grow(q);
	}

	++(q->count);
	n = q->head + q->count - 1;
	if (n >= q->length) {
		n -= q->length;
	}

	memmove(&(q->data[n * (q->chunk)]), x, q->chunk);
}

void cqueue_pop(CQueue *q) {
	if (q->count > 0) {
		++(q->head);
		if (q->head == q->length) {
			q->head = 0;
		}

		--(q->count);
		cqueue_shrink(q);
	}
}

bool cqueue_top_read(CQueue *q, void *x) {
	if (q->count > 0) {
		memmove(x, &(q->data[(q->head) * (q->chunk)]), q->chunk);
		return true;
	} else {
		return false;
	}
}

void *cqueue_top_get(CQueue *q) {
	if (q->count > 0) {
		return &(q->data[(q->head) * (q->chunk)]);
	} else {
		return NULL;
	}
}
