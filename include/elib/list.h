/* list.h - lists. These are doubly-linked, and each node carries a pointer as
 * its payload. Nodes can safely be embedded inside the objects they link, like
 * so:
 * struct foo { struct node n; ... }
 * list_add(&foos, &foo->n, foo)
 */

#ifndef ELIB_LIST_H
#define ELIB_LIST_H

#include <stddef.h>

struct node {
	struct node *prev;
	struct node *next;
	void *data;
};

struct list {
	struct node *head;
	struct node *tail;
	int size;
};

#define LIST_INITIALIZER { NULL, NULL, 0 }

/* Initializes |list| to an empty state. */
void list_init(struct list *list);
/* Adds |node| to |list|, setting |node|'s data pointer to |data|. */
void list_add(struct list *list, struct node *node, void *data);
/* Deletes |node| from |list|, returning |node|'s data pointer. Does not free
 * |node|. */
void *list_del(struct list *list, struct node *node);
/* Returns the |index|th node of |list|. */
struct node *list_get(struct list *list, int index);

/* Iterates across the elements of |h| in order, setting |c| to the current node
 * for each iteration of the loop. */
#define list_foreach(h,c) \
	for ((c) = (h)->head; (c); (c) = (c)->next)

/* Iterates across the elements of |h| in order, safe against deletion. */
#define list_foreach_safe(h,c,t) \
	for ((c) = (h)->head, (t) = (c) ? (c)->next : NULL; \
	     (c); \
	     (c) = (t), (t) = (c) ? (c)->next : NULL)

/* Returns the number of elements in |list|. */
static inline int list_size(struct list *list) { return list->size; }
/* Returns the first node in |list|. */
static inline struct node *list_head(struct list *list) { return list->head; }
static inline struct node *list_tail(struct list *list) { return list->tail; }

void list_check(struct list *list, int forbid_null);
void list_dump(struct list *list);

void list_append(struct list *la, struct list *lb);

#endif /* !ELIB_LIST_H */
