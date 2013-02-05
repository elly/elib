/* elib/smsg.h - bencode implementation */

#ifndef ELIB_SMSG_H
#define ELIB_SMSG_H

#include <elib/buffer.h>
#include <elib/list.h>
#include <elib/map.h>

#include <stdint.h>

enum smsg_type {
	SMSG_STR,
	SMSG_INT,
	SMSG_LIST,
	SMSG_MAP,
};

struct smsg {
	enum smsg_type type;
	struct node node;
	union {
		struct buffer *sval;
		int64_t ival;
		struct list lval;
		struct map *mval;
	} u;
};

int smsg_to(const struct smsg *data, struct buffer *buf);
int smsg_from(struct smsg *data, const struct buffer *buf);

inline struct smsg smsg_str(struct buffer *buf) {
	struct smsg msg;
	msg.type = SMSG_STR;
	msg.u.sval = buf;
	return msg;
}

struct smsg smsg_int(int64_t val) {
	struct smsg msg;
	msg.type = SMSG_INT;
	msg.u.ival = val;
	return msg;
}

#endif /* !ELIB_SMSG_H */
