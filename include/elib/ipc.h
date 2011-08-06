/* ipc.h */

#ifndef IPC_H
#define IPC_H

#include <elib/smsg.h>

typedef struct ipc ipc;
typedef struct ipc_handle ipc_handle;
typedef void (*ipc_func)(void *state, smsg *msg, smsg **reply);

ipc *ipc_new(void);
void ipc_ref(ipc *ipc);
void ipc_unref(ipc *ipc);
int ipc_serve(ipc *ipc, const char *path);
int ipc_hook(ipc *ipc, const char *name, ipc_func fn, void *state);
int ipc_run(ipc *ipc);
int ipc_fd(ipc *ipc);

int ipc_broadcast(ipc *ipc, smsg *msg);

ipc_handle *ipc_connect(ipc *ipc, const char *path);
void ipc_disconnect(ipc_handle *ipc);
int ipc_send(ipc_handle *ipc, smsg *msg);
int ipc_call(ipc_handle *ipc, smsg *msg, smsg **reply);

#endif /* !IPC_H */
