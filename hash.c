#include "hash.h"

// Source: http://www.cse.yorku.ca/~oz/hash.html
HashPrecision hash_buf_length(void *buf, size_t length) {
	HashPrecision *ptr;
	HashPrecision hash = 5381;
	uint8_t *data = buf;
	size_t len;
	size_t mod;

	len = length / ((size_t) sizeof(HashPrecision));

	/* hash * 33 + c */

	ptr = (HashPrecision *) data;
	for (size_t i = 0; i < len; ++i) {
		hash = ((hash << 5) + hash) + ptr[i];
	}

	len *= ((size_t) sizeof(HashPrecision));
	mod = length % ((size_t) sizeof(HashPrecision));
	for (size_t i = 0; i < mod; ++i) {
		hash = ((hash << 5) + hash) + ((HashPrecision) data[len + i]);
	}

	return hash;
}
