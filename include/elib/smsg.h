/* smsg.h */

#ifndef SMSG_H
#define SMSG_H

#include <elib/buffer.h>

enum smsg_type {
	SMSG_NONE,
	SMSG_INT,
	SMSG_UINT,
	SMSG_STR,
	SMSG_SMSG,
};

typedef struct smsg smsg;

smsg *smsg_new(void);
smsg *smsg_frombuf(buffer *buf);
void smsg_ref(smsg *msg);
void smsg_unref(smsg *msg);
int smsg_tobuf(buffer *buf, smsg *msg);

int smsg_addint(smsg *msg, int v);
int smsg_adduint(smsg *msg, unsigned int v);
int smsg_addstr(smsg *msg, const char *v);
int smsg_addsmsg(smsg *msg, smsg *v);
enum smsg_type smsg_gettype(smsg *msg, int index);
int smsg_getint(smsg *msg, int index, int *v);
int smsg_getuint(smsg *msg, int index, unsigned int *v);
int smsg_getstr(smsg *msg, int index, const char **v);
int smsg_getsmsg(smsg *msg, int index, smsg **v);

#endif /* !SMSG_H */
