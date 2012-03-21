/* list.c */

#include <stddef.h>

#include <elib/list.h>

void list_init(struct list *list) {
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
}

void list_add(struct list *list, struct node *node, void *data) {
	node->data = data;
	node->prev = list->tail;
	node->next = NULL;
	if (list->tail)
		list->tail->next = node;
	else
		list->head = node;
	list->tail = node;
	list->size++;
}

void* list_del(struct list *list, struct node *node) {
	if (node->prev)
		node->prev->next = node->next;
	else
		list->head = node->next;
	if (node->next)
		node->next->prev = node->prev;
	else
		list->tail = node->prev;
	list->size--;
	return node->data;
}

struct node *list_get(struct list *list, int index) {
	struct node *n = list->head;
	while (n && index--)
		n = n->next;
	return n;
}
