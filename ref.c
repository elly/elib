/* ref.c */

#include <elib/ref.h>

void ref_init(struct ref *ref, void (*destroy)(struct ref *ref)) {
	ref->destroy = destroy;
	ref->count = 1;
}

void ref_get(struct ref *ref) {
	ref->count++;
}

void ref_put(struct ref *ref) {
	if (!--ref->count)
		ref->destroy(ref);
}
