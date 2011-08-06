#include <elib/ipc.h>

int main(void) {
	ipc *ipc = ipc_new();
	smsg *msg;
	ipc_handle *h = ipc_connect(ipc, "/tmp/ipc");
	msg = smsg_new();
	smsg_addint(msg, 3);
	smsg_addstr(msg, "lolol");
	smsg_adduint(msg, 10);
	ipc_send(h, msg);
	return 0;
}
