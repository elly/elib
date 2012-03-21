/* reactor.h - epoll-based event loop
 * A 'reactor' is an abstraction around an epoll-based event loop. A reactor
 * attaches together zero or more sockets, and waits for events on any of them,
 * calling attached callbacks when events are ready. Events are:
 *   read - this socket has data available to read
 *   write - this socket can be written to
 *   close - this socket became closed
 * To use:
 * First, one creates a reactor |r| with reactor_new(), then adds file
 * descriptors to it (creating sockets) with reactor_add(). After a socket is
 * added, one can set any or all of the desired callbacks in it, then call
 * reactor_refresh() on it, which causes the reactor to listen for those events.
 * To stop listening for an event, set that callback to NULL and call
 * reactor_refresh() again. One then calls reactor_run() to make the reactor
 * dispatch events; reactor_run() will block until it dispatches at least one
 * event. */

#ifndef REACTOR_H
#define REACTOR_H

struct reactor {
	int nsockets;
	int epfd;
};

struct socket {
	int fd;
	struct reactor *r;
	/* You may set everything below this line. */
	void (*read)(struct socket *);
	void (*write)(struct socket *);
	void (*close)(struct socket *);
	void *priv;
};

/* Allocates a new reactor, returning NULL for failure. */
extern struct reactor *reactor_new(void);
/* Frees an existing reactor. There must be no sockets still attached to the
 * reactor. */
extern void reactor_free(struct reactor *r);
/* Adds a file descriptor to the reactor, returning a socket structure for the
 * file descriptor. */
extern struct socket *reactor_add(struct reactor *r, int fd);
/* Refreshes a particular socket |s| in a reactor. This causes events with
 * non-NULL callbacks to be listened for on this socket. */
extern int reactor_refresh(struct reactor *r, struct socket *s);
/* Deletes a particular socket |s| from a reactor, freeing |s|. Does not call
 * |s|->close. */
extern int reactor_del(struct reactor *r, struct socket *s);
/* Dispatches at least one event on |r|, blocking if necessary. */
extern int reactor_run(struct reactor *r);
/* Returns a file descriptor one can select() on to tell when reactor_run() will
 * not block. */
extern int reactor_fd(struct reactor *r);

#endif /* !REACTOR_H */
