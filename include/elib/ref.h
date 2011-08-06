/* ref.h */

#ifndef REF_H
#define REF_H

struct ref {
	int count;
	void (*destroy)(struct ref *ref);
};

void ref_init(struct ref *ref, void (*destroy)(struct ref *ref));
void ref_get(struct ref *ref);
void ref_put(struct ref *ref);

#endif /* !REF_H */
