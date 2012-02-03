/* util.h */

#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdint.h>

/* container_of(p, c, n) -> c which contains p as member n */
#define elems(a) (sizeof(a)/sizeof(a[0]))
#define container_of(p, c, n) ((c *)((void *)p - offsetof(c, n)))
#define unused(x) ((void)(x))

extern void *emalloc(size_t sz);
extern void *erealloc(void *p, size_t sz);
extern void efree(void *p, size_t sz);
extern char *estrdup(const char *s);
extern void estrfree(char *s);

extern void estrlcpy(char *dest, const char *src, size_t sz);
extern void estrlcat(char *dest, const char *src, size_t sz);

extern int emalloc_poison;
extern int emalloc_paranoid;
extern int emalloc_fatal;

#endif /* !UTIL_H */
