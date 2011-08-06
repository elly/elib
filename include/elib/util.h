/* util.h */

#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdint.h>

/* container_of(p, c, n) -> c which contains p as member n */
#define container_of(p, c, n) ((c *)((void *)p - offsetof(c, n)))

#endif /* !UTIL_H */
