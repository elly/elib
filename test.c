#include <stdio.h>

#include <elib/ipc.h>

int main(void) {
	ipc *ipc;
	smsg *msg;
	smsg *reply = NULL;
	ipc_handle *h;
	const char *pong;
	int i;


	for (i = 0; i < 100; i++) {
		ipc = ipc_new();
		h = ipc_connect(ipc, "/tmp/ipc");
		msg = smsg_new();
		smsg_addstr(msg, "ping");
		smsg_addstr(msg, "lolol");
		ipc_handle_call(h, msg, &reply);
		smsg_getstr(reply, 1, &pong);
		printf("pong: %s\n", pong);
	}
	return 0;
}
