/* ref.c */

#include <elib/ref.h>

#include <assert.h>

void ref_init(struct ref *ref, void (*destroy)(struct ref *ref)) {
	ref->destroy = destroy;
	ref->count = 1;
}

void ref_get(struct ref *ref) {
	assert(ref->count >= 0);
	ref->count++;
}

void ref_put(struct ref *ref) {
	assert(ref->count > 0);
	if (!--ref->count)
		ref->destroy(ref);
}
