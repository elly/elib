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

static smsg *sockreadmsg(struct socket *s) {
	ssize_t sz;
	char buf[maxmsglen];
	buffer *b;
	smsg *msg;
	if ((sz = read(s->fd, buf, sizeof(buf))) < 0)
		return NULL;
	b = buffer_newfrom(buf, sz);
	msg = smsg_frombuf(b);
	if (!msg)
		return NULL;
	buffer_free(b);
	return msg;
}

static int sockwritemsg(struct socket *s, smsg *m) {
	buffer *b = buffer_new();
	if (smsg_tobuf(b, m) || buffer_size(b) > maxmsglen) {
		buffer_free(b);
		return -EINVAL;
	}
	write(s->fd, buffer_data(b), buffer_size(b));
	buffer_free(b);
	return 0;
}

static void cliread(struct socket *c) {
	smsg *msg;
	const char *fn = NULL;
	struct node *n;
	struct ipc_hook *h;
	struct ipc *ipc = c->priv;
	smsg *reply = NULL;

	msg = sockreadmsg(c);
	if (!msg)
		return;

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

	if (!reply)
		return;
	sockwritemsg(c, reply);
	smsg_unref(reply);
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
	return sockwritemsg(h->socket, msg);
}

int ipc_call(struct ipc_handle *h, smsg *msg, smsg **reply) {
	int r = ipc_send(h, msg);
	smsg *rep;
	if (r)
		return r;
	rep = sockreadmsg(h->socket);
	if (!rep)
		return -EINVAL;
	*reply = rep;
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
