/* elib/map.h */

#ifndef ELIB_MAP_H
#define ELIB_MAP_H

#include <elib/list.h>

struct map;

extern struct map *map_new();
extern void map_setdestroy(struct map *m, void (*d)(struct map *, const char *, void *));
extern void map_setpriv(struct map *m, void *v);
extern void *map_priv(struct map *m);
extern void map_put(struct map *m, const char *key, void *val);
extern void *map_get(struct map *m, const char *key);

#endif /* !ELIB_MAP_H */
