/* elib/chunkfile.c */

#include <arpa/inet.h> /* htonl */
#include <elib/chunkfile.h>

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

static int fail_missing_mand(struct chunkfile *cf, struct chunkfile_chunk *c)
{
	(void)cf;
	(void)c;
	return 0; /* XXX */
}

struct chunkfile *chunkfile_new()
{
	struct chunkfile *cf = malloc(sizeof *cf);
	if (!cf)
		return NULL;
	cf->f = NULL;
	cf->hooks = NULL;
	cf->unrec_mand = NULL;
	cf->unrec_opt = NULL;
	cf->magic = NULL;
	cf->magicsz = 0;
	cf->priv = NULL;
	if (chunkfile_hook_unrec_mand(cf, fail_missing_mand)) {
		free(cf);
		return NULL;
	}
	return cf;
}

void chunkfile_set_magic(struct chunkfile *cf, const uint8_t *magic, size_t sz)
{
	cf->magic = magic;
	cf->magicsz = sz;
}

static int chunkfile_hook_into(struct chunkfile_hook **hooks, const char *type,
                               chunkfile_cb cb)
{
	struct chunkfile_hook *h = malloc(sizeof *h);
	assert(strlen(type) == CHUNKFILE_TYPESZ);
	if (!h)
		return -ENOMEM;
	memcpy(h->type, type, CHUNKFILE_TYPESZ + 1);
	h->func = cb;
	h->next = *hooks;
	*hooks = h;
	return 0;
}

int chunkfile_hook(struct chunkfile *cf, const char *type, chunkfile_cb cb)
{
	return chunkfile_hook_into(&cf->hooks, type, cb);
}

int chunkfile_hook_unrec_mand(struct chunkfile *cf, chunkfile_cb cb)
{
	return chunkfile_hook_into(&cf->unrec_mand, "****", cb);
}

int chunkfile_hook_unrec_opt(struct chunkfile *cf, chunkfile_cb cb)
{
	return chunkfile_hook_into(&cf->unrec_opt, "****", cb);
}

void chunkfile_set_priv(struct chunkfile *cf, void *priv)
{
	cf->priv = priv;
}

void *chunkfile_priv(struct chunkfile *cf)
{
	return cf->priv;
}

static int chunkfile_parse_one(struct chunkfile *cf,
                               struct chunkfile_chunk *chunk)
{
	uint32_t len;
	uint32_t crc;

	if (fread(&len, sizeof(len), 1, cf->f) != 1)
		return -EINVAL;
	if (fread(chunk->type, CHUNKFILE_TYPESZ, 1, cf->f) != 1)
		return -EINVAL;
	chunk->type[CHUNKFILE_TYPESZ] = '\0';
	chunk->length = htonl(len);
	chunk->offset = ftell(cf->f);
	if (fseek(cf->f, chunk->length, SEEK_CUR))
		return errno;
	if (fread(&crc, sizeof(crc), 1, cf->f) != 1)
		return -EINVAL;
	chunk->crc = htonl(crc);
	return 0;
}

static int chunkfile_run_hooks(struct chunkfile *cf,
                               struct chunkfile_chunk *chunk)
{
	struct chunkfile_hook *h;
	int any_matched = 0;
	for (h = cf->hooks; h; h = h->next) {
		if (!memcmp(h->type, chunk->type, sizeof(h->type))) {
			any_matched = 1;
			if (h->func(cf, chunk))
				return 1;
		}
	}
	if (any_matched)
		return 0;
	if (chunk->type[0] & ('a' ^ 'A'))
		h = cf->unrec_opt;
	else
		h = cf->unrec_mand;
	for (; h; h = h->next)
		if (h->func(cf, chunk))
			return 1;
	return 0;
}

static int check_magic(struct chunkfile *cf)
{
	uint8_t b;
	size_t i;

	for (i = 0; i < cf->magicsz; i++) {
		if (fread(&b, sizeof(b), 1, cf->f) != 1)
			return -EINVAL;
		if (b != cf->magic[i])
			return -EINVAL;
	}
	return 0;
}

int chunkfile_parse(struct chunkfile *cf, FILE *f)
{
	cf->f = f;

	if (check_magic(cf))
		return -EINVAL;

	while (!feof(f) && !ferror(f)) {
		struct chunkfile_chunk c;
		int r = chunkfile_parse_one(cf, &c);
		if (r)
			return r;
		chunkfile_run_hooks(cf, &c);
	}
	return 0;
}

int chunkfile_readchunk(struct chunkfile *cf, struct chunkfile_chunk *c,
                        uint8_t *buf, size_t sz, size_t offset)
{
	size_t cur = ftell(cf->f);
	if (offset >= c->length || offset + sz > c->length || offset + sz < offset)
		return -EINVAL;
	if (fseek(cf->f, c->offset + offset, SEEK_SET)) {
		fseek(cf->f, cur, SEEK_SET);
		return errno;
	}
	if (fread(buf, sz, 1, cf->f) != 1)
		return -EINVAL;
	fseek(cf->f, cur, SEEK_SET);
	return 0;
}


/* ripped from RFC 2083 "PNG Spec 1.0" */
static uint32_t crc_table[256];
static int crc_table_computed = 0;
/* Make the table for a fast CRC. */
static void make_crc_table(void)
{
	uint32_t c;
	int n, k;
	for (n = 0; n < 256; n++) {
		c = n;
		for (k = 0; k < 8; k++) {
			if (c & 1)
				c = 0xedb88320L ^ (c >> 1);
			else
				c = c >> 1;
		}
		crc_table[n] = c;
  	}
	crc_table_computed = 1;
}
/* Update a running CRC with the bytes buf[0..len-1]--the CRC
   should be initialized to all 1's, and the transmitted value
   is the 1's complement of the final running CRC (see the
   crc() routine below)). */

static uint32_t update_crc(uint32_t crc, const uint8_t *buf, size_t len)
{
	uint32_t c = crc;
	size_t n;

	if (!crc_table_computed)
		make_crc_table();
	for (n = 0; n < len; n++)
		c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
	return c;
}

/* Return the CRC of the bytes buf[0..len-1]. */
uint32_t chunkfile_crc32(const uint8_t *buf, size_t len)
{
	return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
}

