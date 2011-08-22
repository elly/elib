#include <elib/ipc.h>
#include <elib/util.h>

void connected(ipc_service *ipc, ipc_handle *newhandle) {
	printf("testd: new handle\n");
}

void disconnected(ipc_service *ipc, ipc_handle *oldhandle) {
	printf("testd: old handle\n");
}

void ping(ipc_handle *handle, smsg *msg, smsg **reply) {
	const char *str;
	smsg *rep;
	if (smsg_getstr(msg, 1, &str))
		return;
	rep = smsg_new();
	if (!rep)
		return;
	smsg_addstr(rep, "pong");
	smsg_addstr(rep, str);
	*reply = rep;
}

int main(void) {
	ipc *ipc;
	ipc_service *srv;

	emalloc_paranoid++;
	emalloc_poison++;
	emalloc_fatal++;

	ipc = ipc_new();
	srv = ipc_serve(ipc, "/tmp/ipc");

	ipc_service_onconnect(srv, connected);
	ipc_service_ondisconnect(srv, disconnected);
	ipc_service_hook(srv, "ping", ping);
	while (1)
		ipc_run(ipc);
	return 0;
}
