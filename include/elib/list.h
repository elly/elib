/* list.h */

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

void list_init(struct list *list);
void list_add(struct list *list, struct node *node, void *data);
void *list_del(struct list *list, struct node *node);
struct node *list_get(struct list *list, int index);

#define list_foreach(h,c) \
	for ((c) = (h)->head; (c); (c) = (c)->next)

static inline int list_size(struct list *list) { return list->size; }
static inline struct node *list_head(struct list *list) { return list->head; }

#endif /* !ELIB_LIST_H */
