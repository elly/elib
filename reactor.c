#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <elib/reactor.h>
#include <elib/util.h>

struct reactor *reactor_new(void) {
	struct reactor *r = emalloc(sizeof *r);
	if (!r)
		return NULL;
	r->epfd = epoll_create1(0);
	if (r->epfd < 0)
		return NULL;
	r->nsockets = 0;
	return r;
}

void reactor_free(struct reactor *r) {
	assert(!r->nsockets);
	close(r->epfd);
	efree(r, sizeof *r);
}

struct socket *reactor_add(struct reactor *r, int fd) {
	struct socket *s = emalloc(sizeof *s);
	struct epoll_event evt;

	if (!s)
		return NULL;

	s->fd = fd;
	s->r = r;
	s->read = NULL;
	s->write = NULL;
	s->close = NULL;
	evt.events = 0;
	evt.data.ptr = s;
	if (epoll_ctl(r->epfd, EPOLL_CTL_ADD, fd, &evt) < 0) {
		efree(s, sizeof *s);
		return NULL;
	}
	r->nsockets++;
	return s;
}

int reactor_refresh(struct reactor *r, struct socket *s) {
	struct epoll_event evt;
	evt.events = 0;
	evt.data.ptr = s;
	if (s->read)
		evt.events |= EPOLLIN;
	if (s->write)
		evt.events |= EPOLLOUT;
	if (s->close)
		evt.events |= EPOLLRDHUP;
	return epoll_ctl(r->epfd, EPOLL_CTL_MOD, s->fd, &evt);
}

int reactor_del(struct reactor *r, struct socket *s) {
	int v = epoll_ctl(r->epfd, EPOLL_CTL_DEL, s->fd, NULL);
	if (v)
		return v;
	printf("free %p\n", s);
	efree(s, sizeof *s);
	r->nsockets--;
	return 0;
}

int reactor_run(struct reactor *r) {
	struct epoll_event evts[16];
	int n;
	int i;
	struct socket *s;

	n = epoll_wait(r->epfd, evts, sizeof(evts) / sizeof(evts[0]), -1);
	if (n < 0)
		return n;
	for (i = 0; i < n; i++) {
		s = evts[i].data.ptr;
		if (evts[i].events & (EPOLLRDHUP | EPOLLERR | EPOLLHUP)) {
			if (s->close)
				s->close(s);
			reactor_del(r, s);
		} else if ((evts[i].events & EPOLLIN) && s->read) {
			s->read(s);
		} else if ((evts[i].events & EPOLLOUT) && s->write) {
			s->write(s);
		}
	}
	return 0;
}

int reactor_fd(struct reactor *r) {
	return r->epfd;
}
