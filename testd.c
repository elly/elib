#include <elib/ipc.h>

void hello(void *state, smsg *msg, smsg **reply) {
	const char *str;
	if (smsg_getstr(msg, 1, &str))
		return;
	printf("hello, %s!\n", str);
}

int main(void) {
	ipc *ipc = ipc_new();
	ipc_serve(ipc, "/tmp/ipc");
	ipc_hook(ipc, "hello", hello, NULL);
	while (1)
		ipc_run(ipc);
	return 0;
}
