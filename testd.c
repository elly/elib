#include <elib/ipc.h>

void ping(void *state, smsg *msg, smsg **reply) {
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
	ipc *ipc = ipc_new();
	ipc_serve(ipc, "/tmp/ipc");
	ipc_hook(ipc, "ping", ping, NULL);
	while (1)
		ipc_run(ipc);
	return 0;
}
