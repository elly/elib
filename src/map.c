/* map.c */

#include <assert.h>
#include <elib/map.h>
#include <elib/util.h>
#include <string.h>

enum {
	HASHSZ = 256
};

/* Jenkins one-at-a-time hash */
static uint32_t _hash(const char *k) {
	uint32_t h = 0;
	while (*k) {
		h += *k;
		h += (h << 10);
		h ^= (h >> 6);
		k++;
	}
	h += (h << 3);
	h ^= (h >> 11);
	h += (h << 15);
	return h;
}

struct entry {
	struct node node;
	const char *key;
	void *val;
};

struct map {
	void (*destroy)(struct map *, const char *, void *);
	struct list nodes[HASHSZ];
	void *priv;
	int iter;
};

struct map *map_new() {
	struct map *m = emalloc(sizeof *m);
	size_t i;
	memset(m, 0, sizeof *m);
	for (i = 0; i < elems(m->nodes); i++)
		list_init(&m->nodes[i]);
	m->iter = 0;
	return m;
}

void map_free(struct map *m) {
	size_t i;
	for (i = 0; i < elems(m->nodes); i++) {
		while (list_size(&m->nodes[i])) {
			struct entry *e = list_head(&m->nodes[i])->data;
			if (m->destroy)
				m->destroy(m, e->key, e->val);
			list_del(&m->nodes[i], &e->node);
			efree(e, sizeof *e);
		}
	}
}

void map_setdestroy(struct map *m, void (*d)(struct map *, const char *, void *)) {
	m->destroy = d;
}

void map_setpriv(struct map *m, void *v) {
	m->priv = v;
}

void *map_priv(struct map *m) {
	return m->priv;
}

void map_put(struct map *m, const char *key, void *val) {
	uint32_t h = _hash(key);
	struct node *n;
	struct entry *e;
	assert(!m->iter);
	list_foreach(&m->nodes[h % HASHSZ], n) {
		e = n->data;
		if (!strcmp(e->key, key)) {
			if (m->destroy)
				m->destroy(m, e->key, e->val);
			e->key = key;
			if (val) {
				e->val = val;
			} else {
				list_del(&m->nodes[h % HASHSZ], &e->node);
				efree(e, sizeof *e);
			}
			return;
		}
	}
	e = emalloc(sizeof *e);
	e->key = key;
	e->val = val;
	list_add(&m->nodes[h % HASHSZ], &e->node, e);
}

void *map_get(struct map *m, const char *key) {
	uint32_t h = _hash(key);
	struct node *n;
	list_foreach(&m->nodes[h % HASHSZ], n) {
		struct entry *e = n->data;
		if (!strcmp(e->key, key))
			return e->val;
	}
	return NULL;
}

void map_each(struct map *m, void (*f)(const char *k, void *v, void *a), void *arg) {
	uint32_t i;
	struct node *n;
	m->iter++;
	for (i = 0; i < elems(m->nodes); i++) {
		list_foreach(&m->nodes[i], n) {
			struct entry *e = n->data;
			f(e->key, e->val, arg);
		}
	}
	m->iter--;
}
