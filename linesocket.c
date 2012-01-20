/* linesocket.c */

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <elib/linesocket.h>
#include <elib/util.h>

static void lsread(struct socket *s) {
	struct linesocket *ls = s->priv;
	char *p;
	ssize_t len;

	len = read(s->fd, ls->rbuf + ls->rbuffill, ls->rbufsize - ls->rbuffill);
	if (len < 0)
		return;
	ls->rbuffill += len;
	while ((p = strstr(ls->rbuf, "\n"))) {
		*p = '\0';
		if (p > ls->rbuf && p[-1] == '\r')
			p[-1] = '\0';
		p++;
		ls->line(ls, ls->rbuf);
		memmove(ls->rbuf, p, ls->rbufsize - (p - ls->rbuf));
		ls->rbuffill -= (p - ls->rbuf);
		memset(ls->rbuf + ls->rbuffill, 0, ls->rbufsize - ls->rbuffill);
	}
}

static void lswrite(struct socket *s) {
	struct linesocket *ls = s->priv;
	ssize_t len;

	len = write(s->fd, ls->wbuf, ls->wbuffill);
	if (len < 0)
		return;
	if ((size_t)len < ls->wbuffill)
		memmove(ls->wbuf, ls->wbuf + len, ls->wbuffill - len);
	ls->wbuffill -= len;
	if (ls->wbuffill)
		return;
	efree(ls->wbuf, ls->wbufsize);
	ls->wbuf = NULL;
	ls->wbufsize = 0;
	s->write = NULL;
	reactor_refresh(s->r, s);
}

static void lsclose(struct socket *s) {
	struct linesocket *ls = s->priv;
	ls->close(ls);
	linesocket_free(ls);
}

struct linesocket *linesocket_new(struct socket *s, size_t rbufmax) {
	struct linesocket *ls = emalloc(sizeof *ls);
	if (!ls)
		return NULL;
	if (!linesocket_init(ls, s, rbufmax))
		return ls;
	efree(ls, sizeof *ls);
	return NULL;
}

int linesocket_init(struct linesocket *ls, struct socket *s, size_t rbufmax) {
	ls->rbuf = emalloc(rbufmax);
	if (!ls->rbuf)
		return 1;
	ls->s = s;
	s->priv = ls;
	ls->line = NULL;
	ls->close = NULL;
	s->read = lsread;
	s->close = lsclose;

	ls->rbufsize = rbufmax;
	ls->rbuffill = 0;
	ls->wbufsize = 0;
	ls->wbuffill = 0;

	ls->priv = NULL;
	reactor_refresh(s->r, s);	/* XXX: failure? */
	return 0;
}

void linesocket_free(struct linesocket *ls) {
	efree(ls->rbuf, ls->rbufsize);
	efree(ls->wbuf, ls->wbufsize);
	efree(ls, sizeof *ls);
}

int linesocket_write(struct linesocket *ls, char *line) {
	if (!ls->wbufsize || ls->wbufsize - ls->wbuffill < strlen(line)) {
		size_t growby = strlen(line) - (ls->wbufsize - ls->wbuffill);
		void *newbuf = erealloc(ls->wbuf, ls->wbufsize + growby);
		if (!newbuf)
			return -ENOMEM;
		ls->wbuf = newbuf;
	}
	memcpy(ls->wbuf + ls->wbuffill, line, strlen(line));
	ls->wbuffill += strlen(line);
	if (!ls->s->write) {
		ls->s->write = lswrite;
		reactor_refresh(ls->s->r, ls->s);
	}
	return 0;
}
