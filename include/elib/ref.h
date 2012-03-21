/* ref.h - reference counts.
 * A reference count can be gotten (increasing its count by 1) and put
 * (decreasing it by 1). Reference counts are always initialized to 1. When a
 * refcount's count hits 0, its destructor is called. The container_of() macro
 * (see /include/elib/util.h) is useful for these.
 */

#ifndef REF_H
#define REF_H

struct ref {
	int count;
	void (*destroy)(struct ref *ref);
};

/* Initialize |ref| to a count of 1 with destructor |destroy|. */
void ref_init(struct ref *ref, void (*destroy)(struct ref *ref));
/* Get a reference to |ref|. Increments |ref|'s count by 1. */
void ref_get(struct ref *ref);
/* Put a reference to |ref|. Decrements |ref|'s count by 1, calling |ref|'s
 * destructor if it reaches 0. */
void ref_put(struct ref *ref);

#endif /* !REF_H */
