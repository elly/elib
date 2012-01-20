/* linesocket.h */

#ifndef LINESOCKET_H
#define LINESOCKET_H

#include <elib/reactor.h>

struct linesocket {
	struct socket *s;

	void (*line)(struct linesocket *, char *);
	void (*close)(struct linesocket *);

	char *rbuf;
	size_t rbufsize;
	size_t rbuffill;
	char *wbuf;
	size_t wbufsize;
	size_t wbuffill;

	void *priv;
};

extern struct linesocket *linesocket_new(struct socket *s, size_t rbufmax);
extern int linesocket_init(struct linesocket *ls, struct socket *s, size_t rbufmax);
extern void linesocket_free(struct linesocket *ls);
extern int linesocket_write(struct linesocket *ls, char *line);

#endif /* !LINESOCKET_H */
