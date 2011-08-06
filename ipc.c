/* ipc.c */

#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <elib/ipc.h>
#include <elib/list.h>
#include <elib/reactor.h>
#include <elib/ref.h>
#include <elib/util.h>

static const size_t maxmsglen = 4096;

struct ipc_handle {
	struct ipc *ipc;
	struct socket *socket;
};

struct ipc_hook {
	struct node node;
	const char *name;
	ipc_func fn;
	void *state;
};

struct ipc {
	struct ref ref;
	struct reactor *reactor;
	struct list hooks;
	struct list handles;
	struct list clients;
};

static void destroyipc(struct ref *ref) {
	struct ipc *ipc = container_of(ref, struct ipc, ref);
	struct node *n;
	while ((n = list_head(&ipc->hooks)))
		free(list_del(&ipc->hooks, n));
	reactor_free(ipc->reactor);
	free(ipc);
}

static void cliread(struct socket *c) {
	char buf[maxmsglen];
	smsg *msg;
	buffer *b;
	const char *fn = NULL;
	struct node *n;
	struct ipc_hook *h;
	struct ipc *ipc = c->priv;
	smsg *reply;

	if (read(c->fd, buf, sizeof(buf)) < 0)
		return;
	b = buffer_newfrom(buf, sizeof(buf));
	msg = smsg_frombuf(b);
	if (!msg)
		return;
	buffer_free(b);

	if (smsg_gettype(msg, 0) != SMSG_STR)
		return;
	if (smsg_getstr(msg, 0, &fn))
		return;

	list_foreach(&ipc->hooks, n) {
		h = n->data;
		if (strcmp(h->name, fn))
			continue;
		h->fn(h->state, msg, &reply);
		break;
	}
}

static void cliclose(struct socket *c) {

}

static void srvread(struct socket *srv) {
	struct ipc *ipc = srv->priv;
	struct socket *c;
	struct sockaddr_un dummy;
	socklen_t dummylen = sizeof(dummy);
	int cfd = accept(srv->fd, (struct sockaddr *)&dummy, &dummylen);
	if (cfd < 0)
		return;
	c = reactor_add(ipc->reactor, cfd);
	if (!c) {
		close(cfd);
		return;
	}
	c->read = cliread;
	c->close = cliclose;
	c->priv = ipc;
	reactor_refresh(ipc->reactor, c);
}

ipc *ipc_new(void) {
	struct ipc *ipc = malloc(sizeof *ipc);
	if (!ipc)
		return NULL;
	ipc->reactor = reactor_new();
	if (!ipc->reactor) {
		free(ipc);
		return NULL;
	}
	ref_init(&ipc->ref, destroyipc);
	list_init(&ipc->hooks);
	return ipc;
}

void ipc_ref(ipc *ipc) {
	ref_get(&ipc->ref);
}

void ipc_unref(ipc *ipc) {
	ref_put(&ipc->ref);
}

int ipc_serve(ipc *ipc, const char *path) {
	struct sockaddr_un sa;
	int sfd;
	int err;
	struct socket *s;

	sfd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (sfd == -1)
		return errno;
	memset(&sa, 0, sizeof sa);
	sa.sun_family = AF_UNIX;
	strncpy(sa.sun_path, path, sizeof(sa.sun_path) - 1);
	if (bind(sfd, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
		err = errno;
		close(sfd);
		return err;
	}

	if (listen(sfd, 5) == -1) {
		err = errno;
		close(sfd);
		return err;
	}

	s = reactor_add(ipc->reactor, sfd);
	if (!s) {
		close(sfd);
		return -ENOMEM;
	}

	s->read = srvread;
	s->priv = ipc;
	reactor_refresh(ipc->reactor, s);

	return 0;
}

int ipc_run(ipc *ipc) {
	return reactor_run(ipc->reactor);
}

int ipc_fd(ipc *ipc) {
	return reactor_fd(ipc->reactor);
}

struct ipc_handle *ipc_connect(struct ipc *ipc, const char *path) {
	struct sockaddr_un sa;
	int fd;
	struct socket *s;
	struct ipc_handle *h;

	h = malloc(sizeof *h);
	if (!h)
		return NULL;

	fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (fd == -1) {
		free(h);
		return NULL;
	}
	memset(&sa, 0, sizeof sa);
	sa.sun_family = AF_UNIX;
	strncpy(sa.sun_path, path, sizeof(sa.sun_path) - 1);
	if (connect(fd, (const struct sockaddr *)&sa, sizeof sa) == -1) {
		free(h);
		close(fd);
		return NULL;
	}

	s = reactor_add(ipc->reactor, fd);
	if (!s) {
		free(h);
		close(fd);
		return NULL;
	}

	h->ipc = ipc;
	h->socket = s;
	s->priv = h;
	reactor_refresh(ipc->reactor, s);
	return h;
}

int ipc_send(struct ipc_handle *h, smsg *msg) {
	int r;
	buffer *b = buffer_new();
	r = smsg_tobuf(b, msg);
	if (r) {
		free(b);
		return r;
	}
	if (buffer_size(b) > maxmsglen) {
		free(b);
		return -E2BIG;
	}
	if (write(h->socket->fd, buffer_data(b), buffer_size(b)) < 0) {
		free(b);
		return errno;
	}
	return 0;
}

int ipc_hook(ipc *ipc, const char *name, ipc_func fn, void *state) {
	struct ipc_hook *h = malloc(sizeof *h);
	if (!h)
		return -ENOMEM;
	h->name = name;
	h->fn = fn;
	h->state = state;
	list_add(&ipc->hooks, &h->node, h);
	return 0;
}
