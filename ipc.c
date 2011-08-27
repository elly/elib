/* ipc.c */

#include <assert.h>
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

#define MAGIC(a,b,c,d) ((a << 24) | (b << 16) | (c << 8) | d)

enum {
	MAG_HOOK = MAGIC('H','O','O','K'),
	MAG_HANDLE = MAGIC('H','N','D','L'),
	MAG_SERVICE = MAGIC('S','R','V','C'),
	MAG_IPC = MAGIC('I','P','C','_'),
};

struct ipc_hook {
	unsigned int magic;
	struct node node;
	const char *name;
	ipc_msg_fn fn;
};

struct ipc_handle {
	unsigned int magic;
	struct node node;
	struct ipc *ipc;
	struct socket *socket;
	struct list hooks;
	struct ipc_service *service;
	void *priv;
};

struct ipc_service {
	unsigned int magic;
	struct ipc *ipc;
	struct socket *socket;
	struct list hooks;
	struct list clients;
	ipc_connect_fn onconnect;
	ipc_disconnect_fn ondisconnect;
	void *priv;
};

struct ipc {
	unsigned int magic;
	struct ref ref;
	struct reactor *reactor;
	struct list handles;
	void *priv;
};

static void destroyipc(struct ref *ref) {
	struct ipc *ipc = container_of(ref, struct ipc, ref);
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

static ipc_msg_fn findfn(struct ipc_handle *handle, const char *name) {
	struct node *n;
	struct ipc_hook *h;

	list_foreach(&handle->hooks, n) {
		h = n->data;
		if (!strcmp(h->name, name))
			return h->fn;
	}

	if (handle->service) {
		list_foreach(&handle->service->hooks, n) {
			h = n->data;
			if (!strcmp(h->name, name))
				return h->fn;
		}
	}

	if (strcmp(name, IPC_UNKNOWN))
		return findfn(handle, IPC_UNKNOWN);
	return NULL; 
}

static void cliread(struct socket *c) {
	smsg *msg;
	const char *name = NULL;
	struct ipc_handle *handle = c->priv;
	smsg *reply = NULL;
	ipc_msg_fn fn;

	assert(handle->magic == MAG_HANDLE);
	if (handle->service)
		assert(handle->service->magic == MAG_SERVICE);

	msg = sockreadmsg(c);
	if (!msg)
		return;

	if (smsg_gettype(msg, 0) != SMSG_STR)
		return;
	if (smsg_getstr(msg, 0, &name))
		return;

	fn = findfn(handle, name);
	if (!fn)
		return;

	fn(handle, msg, &reply);
	smsg_unref(msg);

	if (!reply)
		return;
	sockwritemsg(c, reply);
	smsg_unref(reply);
}

static void cliclose(struct socket *c) {
	struct ipc_handle *handle = c->priv;
	struct ipc_service *service = handle->service;

	assert(handle->magic == MAG_HANDLE);
	assert(service->magic == MAG_SERVICE);

	if (service->ondisconnect)
		service->ondisconnect(service, handle);
	list_del(&service->clients, &handle->node);
	/* XXX: leak handle resources */
	efree(handle, sizeof *handle);
}

static void srvread(struct socket *sock) {
	struct ipc_service *srv = sock->priv;
	struct socket *c;
	struct sockaddr_un dummy;
	struct ipc_handle *h;
	socklen_t dummylen = sizeof(dummy);
	int cfd = accept(sock->fd, (struct sockaddr *)&dummy, &dummylen);

	assert(srv->magic == MAG_SERVICE);
	assert(srv->socket == sock);

	h = emalloc(sizeof *h);
	if (!h)
		return;
	memset(h, 0xCC, sizeof *h);

	if (cfd < 0) {
		efree(h, sizeof *h);
		return;
	}
	c = reactor_add(srv->ipc->reactor, cfd);
	if (!c) {
		efree(h, sizeof *h);
		close(cfd);
		return;
	}
	h->magic = MAG_HANDLE;
	h->ipc = srv->ipc;
	h->service = srv;
	h->socket = c;
	list_add(&srv->clients, &h->node, h);
	list_init(&h->hooks);
	c->read = cliread;
	c->close = cliclose;
	c->priv = h;
	reactor_refresh(srv->ipc->reactor, c);
	if (srv->onconnect)
		srv->onconnect(srv, h);
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
	ipc->magic = MAG_IPC;
	return ipc;
}

void ipc_ref(ipc *ipc) {
	ref_get(&ipc->ref);
}

void ipc_unref(ipc *ipc) {
	ref_put(&ipc->ref);
}

int ipc_run(struct ipc *ipc) {
	return reactor_run(ipc->reactor);
}

int ipc_fd(struct ipc *ipc) {
	return reactor_fd(ipc->reactor);
}

void ipc_setpriv(struct ipc *ipc, void *priv) {
	ipc->priv = priv;
}

void *ipc_priv(struct ipc *ipc) {
	return ipc->priv;
}

struct ipc_service *ipc_serve(struct ipc *ipc, const char *path) {
	struct sockaddr_un sa;
	int sfd;
	struct socket *s;
	struct ipc_service *sv;

	sv = malloc(sizeof *sv);
	if (!sv)
		return NULL;

	sfd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (sfd == -1)
		goto free;
	memset(&sa, 0, sizeof sa);
	sa.sun_family = AF_UNIX;
	strncpy(sa.sun_path, path, sizeof(sa.sun_path) - 1);
	if (bind(sfd, (struct sockaddr *)&sa, sizeof(sa)) == -1)
		goto close;

	if (listen(sfd, 5) == -1)
		goto close;

	s = reactor_add(ipc->reactor, sfd);
	if (!s)
		goto close;

	s->read = srvread;
	s->priv = sv;
	sv->onconnect = NULL;
	sv->ondisconnect = NULL;
	sv->socket = s;
	sv->ipc = ipc;
	list_init(&sv->hooks);
	list_init(&sv->clients);
	reactor_refresh(ipc->reactor, s);
	ipc_ref(ipc);
	sv->magic = MAG_SERVICE;

	return sv;
close:
	close(sfd);
free:
	free(sv);
	return NULL;
}

void ipc_service_unserve(struct ipc_service *srv) {
	reactor_del(srv->ipc->reactor, srv->socket);
	/* XXX leak clients, hooks */
	ipc_unref(srv->ipc);
	free(srv);
}

struct ipc *ipc_service_ipc(struct ipc_service *srv) {
	return srv->ipc;
}

int ipc_service_hook(struct ipc_service *srv, const char *name, ipc_msg_fn fn) {
	struct ipc_hook *h = malloc(sizeof *h);
	if (!h)
		return -ENOMEM;
	memset(h, 0xCF, sizeof *h);
	h->fn = fn;
	h->name = name;
	list_add(&srv->hooks, &h->node, h);
	h->magic = MAG_HOOK;
	return 0;
}

void ipc_service_onconnect(struct ipc_service *srv, ipc_connect_fn fn) {
	srv->onconnect = fn;
}

void ipc_service_ondisconnect(struct ipc_service *srv, ipc_disconnect_fn fn) {
	srv->ondisconnect = fn;
}

void ipc_service_setpriv(struct ipc_service *srv, void *priv) {
	srv->priv = priv;
}

void *ipc_service_priv(ipc_service *srv) {
	return srv->priv;
}

int ipc_service_broadcast(struct ipc_service *srv, smsg *msg) {
	struct ipc_handle *h;
	struct node *n;
	int r = 0;
	int nr;

	list_foreach(&srv->clients, n) {
		h = n->data;
		nr = ipc_handle_send(h, msg);
		if (!r)
			r = nr;
	}

	return r;
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
	h->service = NULL;
	list_init(&h->hooks);
	s->priv = h;
	s->read = cliread;
	s->close = cliclose;
	reactor_refresh(ipc->reactor, s);
	h->magic = MAG_HANDLE;
	return h;
}

void ipc_handle_disconnect(struct ipc_handle *handle) {
	close(handle->socket->fd);
}

struct ipc_service *ipc_handle_service(struct ipc_handle *h) {
	return h->service;
}

int ipc_handle_send(struct ipc_handle *h, smsg *msg) {
	return sockwritemsg(h->socket, msg);
}

int ipc_handle_call(struct ipc_handle *h, smsg *msg, smsg **reply) {
	int r = ipc_handle_send(h, msg);
	smsg *rep;
	if (r)
		return r;
	rep = sockreadmsg(h->socket);
	if (!rep)
		return -EINVAL;
	*reply = rep;
	return 0;
}

int ipc_handle_hook(ipc_handle *handle, const char *name, ipc_msg_fn fn) {
	struct ipc_hook *h = malloc(sizeof *h);
	if (!h)
		return -ENOMEM;
	h->name = name;
	h->fn = fn;
	list_add(&handle->hooks, &h->node, h);
	return 0;
}

void ipc_handle_setpriv(ipc_handle *handle, void *priv) {
	handle->priv = priv;
}

void *ipc_handle_priv(ipc_handle *handle) {
	return handle->priv;
}
