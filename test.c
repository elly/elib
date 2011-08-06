#include <elib/ipc.h>

int main(void) {
	ipc *ipc = ipc_new();
	smsg *msg;
	ipc_handle *h = ipc_connect(ipc, "/tmp/ipc");
	msg = smsg_new();
	smsg_addstr(msg, "hello");
	smsg_addstr(msg, "world");
	ipc_send(h, msg);
	return 0;
}
