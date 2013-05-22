/* list.c */

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <elib/list.h>

enum {
	POISON_PREV = 0xAB,
	POISON_NEXT = 0xAC,
};

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
	memset(&node->prev, POISON_PREV, sizeof(node->prev));
	memset(&node->next, POISON_NEXT, sizeof(node->next));
	return node->data;
}

struct node *list_get(struct list *list, int index) {
	struct node *n = list->head;
	while (n && index--)
		n = n->next;
	return n;
}

void list_check(struct list *list, int forbid_null) {
	struct node *n;
	int count = 0;

	for (n = list->head; n; n = n->next) {
		assert(!n->next || n->next->prev == n);
		assert(!n->prev || n->prev->next == n);
		assert(n->next || list->tail == n);
		assert(n->prev || list->head == n);
		if (forbid_null)
			assert(n->data);
		count++;
	}

	assert(count == list->size);
}

void list_print(struct list *list) {
	struct node *n;

	printf("list %p<->%p\n", list->head, list->tail);
	for (n = list->head; n; n = n->next)
		printf("  %p <--- %p[%p] ---> %p\n", n->prev, n, n->data, n->next);
}

void list_append(struct list *la, struct list *lb) {
	if (!la->head) {
		la->head = lb->head;
	} else {
		la->tail->next = lb->head;
		lb->head->prev = la->tail;
	}
	la->tail = lb->tail;
	la->size += lb->size;
}
