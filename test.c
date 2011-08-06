#include <stdio.h>

#include <elib/ipc.h>

int main(void) {
	ipc *ipc = ipc_new();
	smsg *msg;
	smsg *reply = NULL;
	ipc_handle *h = ipc_connect(ipc, "/tmp/ipc");
	const char *pong;
	msg = smsg_new();
	smsg_addstr(msg, "ping");
	smsg_addstr(msg, "lolol");
	ipc_call(h, msg, &reply);
	smsg_getstr(reply, 1, &pong);
	printf("pong: %s\n", pong);
	return 0;
}
