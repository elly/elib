/* util.h */

#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdint.h>

/* Returns the number of elements in array |a|. */
#define elems(a) (sizeof(a)/sizeof(a[0]))
/* container_of(p, c, n) -> c which contains p as member n. */
#define container_of(p, c, n) ((c *)((void *)p - offsetof(c, n)))
/* Declares a value unused. */
#define unused(x) ((void)(x))

/* Allocates a block of memory of size |sz|. */
extern void *emalloc(size_t sz);
/* Reallocates an existing block of memory |p| to be of size |sz|. */
extern void *erealloc(void *p, size_t sz);
/* Frees an existing block of memory |p| of size |sz|. Noop for NULL |p|. */
extern void efree(void *p, size_t sz);
/* Returns a newly-allocated copy of a string |s|. */
extern char *estrdup(const char *s);
/* Frees a string |s| previously allocated with estrdup. Noop for NULL |s|. */
extern void estrfree(char *s);

/* Copies a string of at most |sz| bytes (including the null terminator) from
 * |src| to |dest|. Always null-terminates. */
extern void estrlcpy(char *dest, const char *src, size_t sz);
/* Appends a string |src| to |dest|, but never stores more than |sz| bytes at
 * |dest|, including null-termination. Truncates if necessary. */
extern void estrlcat(char *dest, const char *src, size_t sz);

/* If not zero, emalloc/erealloc will poison newly-allocated memory. */
extern int emalloc_poison;
/* If not zero, emalloc/erealloc/estrdup will insert red zones around
 * allocations to catch heap overflows and underflows. */
extern int emalloc_paranoid;
/* If not zero, emalloc/erealloc/estrdup will abort() if allocation fails. */
extern int emalloc_fatal;

#endif /* !UTIL_H */
