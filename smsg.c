/* smsg.c */

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <elib/buffer.h>
#include <elib/list.h>
#include <elib/ref.h>
#include <elib/smsg.h>
#include <elib/util.h>

struct smsg_val {
	enum smsg_type type;
	struct node node;
	union {
		int iv;
		unsigned int uv;
		char *sv;
		smsg *mv;
	} u;
};

struct smsg {
	struct ref ref;
	struct list vals;
};

static struct smsg_val *valnew(void) {
	struct smsg_val *v = malloc(sizeof *v);
	if (v)
		memset(v, 0, sizeof *v);
	return v;
}

static void valfree(struct smsg_val *v) {
	if (v->u.sv)
		free(v->u.sv);
	if (v->u.mv)
		smsg_unref(v->u.mv);
	free(v);
}

static void destroysmsg(struct ref *ref) {
	smsg *msg = container_of(ref, smsg, ref);
	struct node *node;
	while ((node = list_head(&msg->vals)))
		valfree(list_del(&msg->vals, node));
}

smsg *smsg_new(void) {
	smsg *m = malloc(sizeof *m);
	if (!m)
		return NULL;
	list_init(&m->vals);
	ref_init(&m->ref, destroysmsg);
	return m;
}

int smsg_addint(smsg *msg, int v) {
	struct smsg_val *n = valnew();
	if (!n)
		return -ENOMEM;
	n->type = SMSG_INT;
	n->u.iv = v;
	list_add(&msg->vals, &n->node, n);
	return 0;
}

int smsg_adduint(smsg *msg, unsigned int v) {
	struct smsg_val *n = valnew();
	if (!n)
		return -ENOMEM;
	n->type = SMSG_UINT;
	n->u.uv = v;
	list_add(&msg->vals, &n->node, n);
	return 0;
}

int smsg_addstr(smsg *msg, const char *v) {
	struct smsg_val *n = valnew();
	if (!n)
		return -ENOMEM;
	n->u.sv = strdup(v);
	if (!n->u.sv) {
		valfree(n);
		return -ENOMEM;
	}
	n->type = SMSG_STR;
	list_add(&msg->vals, &n->node, n);
	return 0;
}

int smsg_addsmsg(smsg *msg, smsg *v) {
	struct smsg_val *n = valnew();
	if (!n)
		return -ENOMEM;
	smsg_ref(v);
	n->u.mv = v;
	n->type = SMSG_SMSG;
	list_add(&msg->vals, &n->node, n);
	return 0;
}

static struct smsg_val *valget(smsg *msg, int index) {
	struct node *n = list_get(&msg->vals, index);
	return n ? n->data : NULL;
}

enum smsg_type smsg_gettype(smsg *msg, int index) {
	struct smsg_val *v = valget(msg, index);
	return v ? v->type : SMSG_NONE;
}

int smsg_getint(smsg *msg, int index, int *v) {
	struct smsg_val *v0 = valget(msg, index);
	if (!v0 || v0->type != SMSG_INT)
		return -EINVAL;
	*v = v0->u.iv;
	return 0;
}

int smsg_getuint(smsg *msg, int index, unsigned int *v) {
	struct smsg_val *v0 = valget(msg, index);
	if (!v0 || v0->type != SMSG_UINT)
		return -EINVAL;
	*v = v0->u.uv;
	return 0;
}

int smsg_getstr(smsg *msg, int index, const char **v) {
	struct smsg_val *v0 = valget(msg, index);
	if (!v0 || v0->type != SMSG_STR)
		return -EINVAL;
	*v = v0->u.sv;
	return 0;
}

int smsg_getsmsg(smsg *msg, int index, smsg **v) {
	struct smsg_val *v0 = valget(msg, index);
	if (!v0 || v0->type != SMSG_SMSG)
		return -EINVAL;
	smsg_ref(v0->u.mv);
	*v = v0->u.mv;
	return 0;
}

void smsg_ref(smsg *msg) {
	ref_get(&msg->ref);
}

void smsg_unref(smsg *msg) {
	ref_put(&msg->ref);
}

static int valtobuf(buffer *b, struct smsg_val *v) {
	char buf[32];
	int r = 0;
	switch (v->type) {
		case SMSG_NONE:
			assert(0);
			return -1;
		case SMSG_INT:
			snprintf(buf, sizeof(buf), "i%d", v->u.iv);
			return buffer_appends(b, buf);
		case SMSG_UINT:
			snprintf(buf, sizeof(buf), "u%u", v->u.uv);
			return buffer_appends(b, buf);
		case SMSG_STR:
			snprintf(buf, sizeof(buf), "s%zu:", strlen(v->u.sv));
			r |= buffer_appends(b, buf);
			r |= buffer_appends(b, v->u.sv);
			return r;
		case SMSG_SMSG:
			return smsg_tobuf(b, v->u.mv);
	}
	assert(0);
	return -1;
}

int smsg_tobuf(buffer *buf, smsg *msg) {
	struct node *n;
	struct smsg_val *v;
	int r = 0;

	r |= buffer_appends(buf, "l");
	list_foreach(&msg->vals, n) {
		v = n->data;
		r |= valtobuf(buf, v);
	}
	r |= buffer_appends(buf, "e");
	if (!r)
		return 0;
	buffer_free(buf);
	return r;
}
