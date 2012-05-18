/* elib/map.h - string->void* maps */

#ifndef ELIB_MAP_H
#define ELIB_MAP_H

#include <elib/list.h>

struct map;

/* Allocates a new, empty map. */
extern struct map *map_new();

/* Frees |m|, calling the destructor on every (key, value) pair in it. */
extern void map_free(struct map *m);

/* Sets the destructor for |m|. The destructor is called on an element |k, v| of
 * |m| whenever it is removed, either through map_put(m, k, v0) or map_free(m).
 * The destructor is passed the map itself and the key and value of the element
 * being destroyed. */
extern void map_setdestroy(struct map *m, void (*d)(struct map *, const char *, void *));

/* Sets the private data of |m| to |v|. See /README. */
extern void map_setpriv(struct map *m, void *v);

/* Returns the private data of |m|. See /README. */
extern void *map_priv(struct map *m);

/* Stores value |val| for key |key| in map |m|. If |val| is NULL, deletes key
 * |key| from |m|. Any existing value for |key| is destroyed by calling the
 * destructor on it (see map_setdestroy()). Does not copy |key|; it is the
 * caller's responsibility to ensure that |key| lives at least as long as the
 * map entry. */
extern void map_put(struct map *m, const char *key, void *val);

/* Returns the value corresponding to |key| in |m|. */
extern void *map_get(struct map *m, const char *key);

/* Calls |f| on each element of |m|. |f| is called with the key and value of
 * each element and |arg|. |f| must not call map_put(m, ...). */
extern void map_each(struct map *m, void (*f)(const char *k, void *v, void *a), void *arg);

#endif /* !ELIB_MAP_H */
