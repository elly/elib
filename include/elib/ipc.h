/* ipc.h */

#ifndef IPC_H
#define IPC_H

#include <elib/smsg.h>

#define IPC_UNKNOWN	"ipc.unknown"

typedef struct ipc ipc;
typedef struct ipc_service ipc_service;
typedef struct ipc_handle ipc_handle;
typedef void (*ipc_msg_fn)(ipc_handle *handle, smsg *msg, smsg **reply);
typedef void (*ipc_connect_fn)(ipc_service *ipc, ipc_handle *newhandle);
typedef void (*ipc_disconnect_fn)(ipc_service *ipc, ipc_handle *oldhandle);

ipc *ipc_new(void);
void ipc_ref(ipc *ipc);
void ipc_unref(ipc *ipc);
int ipc_run(ipc *ipc);
int ipc_fd(ipc *ipc);
void ipc_setpriv(ipc *ipc, void *priv);
void *ipc_priv(ipc *ipc);

ipc_service *ipc_serve(ipc *ipc, const char *path);
void ipc_service_unserve(ipc_service *srv);
ipc *ipc_service_ipc(ipc_service *srv);
int ipc_service_hook(ipc_service *srv, const char *name, ipc_msg_fn fn);
void ipc_service_onconnect(ipc_service *srv, ipc_connect_fn fn);
void ipc_service_ondisconnect(ipc_service *srv, ipc_disconnect_fn fn);
int ipc_service_broadcast(ipc_service *srv, smsg *msg);
void ipc_service_setpriv(ipc_service *srv, void *priv);
void *ipc_service_priv(ipc_service *srv);

ipc_handle *ipc_connect(ipc *ipc, const char *path);
void ipc_handle_disconnect(ipc_handle *handle);
ipc_service *ipc_handle_service(ipc_handle *handle);
int ipc_handle_send(ipc_handle *handle, smsg *msg);
int ipc_handle_call(ipc_handle *handle, smsg *msg, smsg **reply);
int ipc_handle_hook(ipc_handle *handle, const char *name, ipc_msg_fn fn);
void ipc_handle_setpriv(ipc_handle *handle, void *priv);
void *ipc_handle_priv(ipc_handle *handle);

#endif /* !IPC_H */
