#ifndef REACTOR_H
#define REACTOR_H

struct reactor {
	int nsockets;
	int epfd;
};

struct socket {
	int fd;
	struct reactor *r;
	void (*read)(struct socket *);
	void (*write)(struct socket *);
	void (*close)(struct socket *);
	void *priv;
};

extern struct reactor *reactor_new(void);
extern void reactor_free(struct reactor *r);
extern struct socket *reactor_add(struct reactor *r, int fd);
extern int reactor_refresh(struct reactor *r, struct socket *s);
extern int reactor_del(struct reactor *r, struct socket *s);
extern int reactor_run(struct reactor *r);
extern int reactor_fd(struct reactor *r);

#endif /* !REACTOR_H */
