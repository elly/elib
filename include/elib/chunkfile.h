/* elib/chunkfile.h - chunkfile library
 * A chunkfile is a data stream with an optional magic number header, then one
 * or more chunks, each of which are all composed of 4 bytes of length, 4 bytes
 * of type, n bytes of data, then 4 bytes of CRC. This is basically a
 * generalization of the PNG container format, so see RFC 2083. */

#ifndef ELIB_CHUNKFILE_H
#define ELIB_CHUNKFILE_H

#include <stdio.h>
#include <stdint.h>

enum {
	CHUNKFILE_TYPESZ = 4
};

struct chunkfile_chunk {
	char type[CHUNKFILE_TYPESZ + 1];
	size_t length;
	size_t offset;
	uint32_t crc;
};

struct chunkfile;

typedef int (*chunkfile_cb)(struct chunkfile *cf, struct chunkfile_chunk *c);

struct chunkfile_hook {
	char type[CHUNKFILE_TYPESZ + 1];
	chunkfile_cb func;
	struct chunkfile_hook *next;
};

struct chunkfile {
	FILE *f;
	struct chunkfile_hook *hooks;
	struct chunkfile_hook *unrec_mand;
	struct chunkfile_hook *unrec_opt;
	const uint8_t *magic;
	size_t magicsz;
	void *priv;
};

struct chunkfile *chunkfile_new(void);
void chunkfile_set_magic(struct chunkfile *cf, const uint8_t *magic, size_t sz);
int chunkfile_hook(struct chunkfile *cf, const char *type, chunkfile_cb cb);
int chunkfile_hook_unrec_mand(struct chunkfile *cf, chunkfile_cb cb);
int chunkfile_hook_unrec_opt(struct chunkfile *cf, chunkfile_cb cb);
void chunkfile_set_priv(struct chunkfile *cf, void *priv);
void *chunkfile_priv(struct chunkfile *cf);
int chunkfile_parse(struct chunkfile *cf, FILE *f);
int chunkfile_readchunk(struct chunkfile *cf, struct chunkfile_chunk *c,
                        uint8_t *buf, size_t sz, size_t offset);
void chunkfile_free(struct chunkfile *cf);

uint32_t chunkfile_crc32(const uint8_t *buf, size_t sz);

#endif /* !ELIB_CHUNKFILE_H */
