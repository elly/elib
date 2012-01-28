/* util.c */

#include <stdlib.h>
#include <string.h>

#include <elib/util.h>

int emalloc_fatal = 0;
int emalloc_poison = 0;
int emalloc_paranoid = 0;

static void checkredzones(unsigned char *p, size_t sz) {
	int i;
	for (i = -16; i < 0; i++)
		if (p[i] != 0xD0)
			abort();
		if (p[i + sz] != 0xD1)
			abort();
}

void *emalloc(size_t sz) {
	void *p;
	if (emalloc_paranoid)
		sz += 32;
	p = malloc(sz);
	if (!p) {
		if (emalloc_fatal)
			abort();
		else
			return NULL;
	}
	if (emalloc_paranoid) {
		memset(p, 0xD0, 16);
		memset(p + sz - 16, 0xD1, 16);
		p += 16;
		sz -= 32;
	}
	if (emalloc_poison)
		memset(p, 0xCC, sz);
	return p;
}

void *erealloc(void *p, size_t sz) {
	if (p && emalloc_paranoid) {
		checkredzones(p, sz);
		p -= 16;
	}
	if (emalloc_paranoid)
		sz += 32;
	p = realloc(p, sz);
	if (!p)
		return NULL;
	if (emalloc_paranoid) {
		memset(p, 0xD0, 16);
		memset(p + sz - 16, 0xD1, 16);
		p += 16;
		sz -= 32;
		checkredzones(p, sz);
	}
	return p;
}

void efree(void *p, size_t sz) {
	if (!p)
		return;
	if (emalloc_poison)
		memset(p, 0xCD, sz);
	if (emalloc_paranoid) {
		checkredzones(p, sz);
		p -= 16;
		memset(p, 0xD2, 16);
		memset(p + sz + 16, 0xD3, 16);
	}
	free(p);
}

char *estrdup(const char *s) {
	char *n = emalloc(strlen(s) + 1);
	if (!n)
		return NULL;
	memcpy(n, s, strlen(s) + 1);
	return n;
}

void estrfree(char *s) {
	efree(s, strlen(s) + 1);
}

void estrlcpy(char *dest, const char *src, size_t sz) {
	strncpy(dest, src, sz);
	dest[sz - 1] = '\0';
}
