#include "memory.h"

MemoryRecoveryCallback _recovery = NULL;

// Attempts to allocate memory safely. If fails, instead of returning NULL, it
// calls for the recovery function in an attempt to recover from bad allocation.
void *memory_malloc(size_t n) {
	void *p = malloc(n);
	if (p == NULL) {
		if (_recovery == NULL) {
			exit(0);
		} else {
			return _recovery(n);
		}
	} else {
		return p;
	}
}

// Same as free, but doesn't return anything.
void memory_free(void *p) {
	free(p);
}

// By default, there's no recovery call set and the program will simply exit.
void memory_set_recovery_callback(MemoryRecoveryCallback callback) {
	_recovery = callback;
}
