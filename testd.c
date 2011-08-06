#include <elib/ipc.h>

int main(void) {
	ipc *ipc = ipc_new();
	ipc_serve(ipc, "/tmp/ipc");
	while (1)
		ipc_run(ipc);
	return 0;
}
