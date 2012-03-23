/* linesocket.h - line-buffered sockets
 * These wrap around ordinary reactor sockets to add line-buffering for both
 * input and output. If you wrap a socket in a linesocket, you may never touch
 * any members of the underlying socket, including its priv pointer; use the
 * linesocket's priv pointer and hooks instead. These sockets have a maximum
 * buffer size; if a write would exceed their maximum buffer size, they return
 * -ENOMEM, and if a read would exceed their maximum buffer size, it is
 * truncated (and remaining data left in the socket buffer). */

#ifndef LINESOCKET_H
#define LINESOCKET_H

#include <elib/reactor.h>
#include <stddef.h>

struct linesocket {
	struct socket *s;

	void (*line)(struct linesocket *, char *);
	void (*close)(struct linesocket *);
	void (*writedone)(struct linesocket *);

	char *rbuf;
	size_t rbufsize;
	size_t rbuffill;
	char *wbuf;
	size_t wbufsize;
	size_t wbuffill;

	void *priv;
};

/* Allocate a new line-buffered socket wrapping |s|, which will buffer a maximum
 * of |rbufmax| bytes. */
extern struct linesocket *linesocket_new(struct socket *s, size_t rbufmax);
/* Frees an existing linesocket, but does not close or destroy the underlying
 * socket. */
extern void linesocket_free(struct linesocket *ls);
/* Writes a line to the given linesocket, filling its buffer as necessary;
 * returns -ENOMEM if maximum buffer size is exceeded. */
extern int linesocket_write(struct linesocket *ls, const char *line);
/* Refreshes a linesocket, enabling or disabling readiness to read as needed. */
extern int linesocket_refresh(struct linesocket *ls);
/* Returns whether there is any buffered output left. */
extern int linesocket_hasoutput(struct linesocket *ls);

#endif /* !LINESOCKET_H */
