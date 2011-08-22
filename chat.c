/* chat.c */

#include <elib/ipc.h>
#include <elib/reactor.h>

void msg(ipc_handle *handle, smsg *msg, smsg **reply) {
	printf("msg!\n");
}

int main(void) {
	ipc *ipc;
	ipc_handle *h;
	smsg *m;

	ipc = ipc_new();
	h = ipc_connect(ipc, "/tmp/chat");
	m = smsg_new();

	smsg_addstr(m, "msg");
	smsg_addstr(m, "???");

	ipc_handle_hook(h, "msg", msg);
	ipc_handle_send(h, m);
	while (1)
		ipc_run(ipc);
	return 0;
}
