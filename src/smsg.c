/* smsg.c - bencode implementation */

#include <elib/smsg.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

static int smsg_str_to(const struct buffer *str, struct buffer *buf) {
	size_t sz = buffer_size(str);
	void *d = buffer_data(str);
	char abuf[32];
	int r;
	snprintf(abuf, sizeof(abuf), "%lu", sz);
	r = buffer_appends(buf, abuf);
	r |= buffer_appends(buf, ":");
	r |= buffer_append(buf, d, sz);
	return r;
}

static int smsg_int_to(int64_t intv, struct buffer *buf) {
	int r;
	char abuf[32];
	snprintf(abuf, sizeof(abuf), "%ld", intv);
	r = buffer_appends(buf, "i");
	r |= buffer_appends(buf, abuf);
	r |= buffer_appends(buf, "e");
	return r;
}

static int smsg_list_to(const struct list *lval, struct buffer *buf) {
	struct node *n;
	int r;

	r = buffer_appends(buf, "l");
	list_foreach(lval, n) {
		r |= smsg_to(n->data, buf);
		if (r)
			return r;
	}
	r |= buffer_appends(buf, "e");
	return 0;
}

struct encode_map {
	struct buffer *buf;
	int r;
};

static void smsg_mapelem_to(const char *key, void *val, void *arg) {
	struct encode_map *em = arg;
	struct buffer *b;
	if (em->r)
		return;
	b = buffer_newfrom(key, strlen(key));
	if (!b) {
		em->r = -ENOMEM;
		return;
	}
	em->r |= smsg_str_to(b, em->buf);
	em->r |= smsg_to(val, em->buf);
	buffer_free(b);
}

static int smsg_map_to(const struct map *map, struct buffer *buf) {
	struct encode_map em;
	em.buf = buf;
	em.r = 0;
	em.r |= buffer_appends(buf, "d");
	map_each((struct map *)map, smsg_mapelem_to, &em);
	em.r |= buffer_appends(buf, "e");
	return em.r;
}

int smsg_to(const struct smsg *data, struct buffer *buf) {
	switch (data->type) {
		case SMSG_STR:
			return smsg_str_to(data->u.sval, buf);
		case SMSG_INT:
			return smsg_int_to(data->u.ival, buf);
		case SMSG_LIST:
			return smsg_list_to(&data->u.lval, buf);
		case SMSG_MAP:
			return smsg_map_to(data->u.mval, buf);
		default:
			assert(0);
	}
	return -1;
}

int smsg_from(struct smsg *data, const struct buffer *buf) {
	
}
