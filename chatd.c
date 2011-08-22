/* chatd.c */

#include <elib/ipc.h>
#include <elib/util.h>

void msg(ipc_handle *handle, smsg *msg, smsg **reply) {
	unused(reply);
	ipc_service_broadcast(ipc_handle_service(handle), msg);
}

int main(void) {
	ipc *ipc;
	ipc_service *srv;

	emalloc_paranoid++;
	emalloc_poison++;
	emalloc_fatal++;

	ipc = ipc_new();
	srv = ipc_serve(ipc, "/tmp/chat");

	ipc_service_hook(srv, "msg", msg);
	while (1)
		ipc_run(ipc);
	return 0;
}
