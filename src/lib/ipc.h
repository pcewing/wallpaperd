#ifndef IPC_H
#define IPC_H

#include "core.h"

/* IpcMessage Interface */

struct ipc_msg_t;

typedef struct {
    wpd_error_t (*const create)(struct ipc_msg_t **msg);
    wpd_error_t (*const destroy)(struct ipc_msg_t **msg);
    uint32_t (*const size)(struct ipc_msg_t *msg);
    uint8_t *(*const data)(struct ipc_msg_t *msg);
} _IpcMessage;

extern _IpcMessage const IpcMessage;

/* IpcServer Interface */

struct ipc_server_t;

typedef struct {
    wpd_error_t (*const create)(struct ipc_server_t **server);
    wpd_error_t (*const destroy)(struct ipc_server_t **server);
    wpd_error_t (*const start)(struct ipc_server_t *server);
    wpd_error_t (*const stop)(struct ipc_server_t *server);
    wpd_error_t (*const poll)(struct ipc_server_t *server, bool *accepted,
                              struct ipc_msg_t *msg);
    wpd_error_t (*const send)(struct ipc_server_t *server, size_t byte_count,
                              void *bytes);
    wpd_error_t (*const close)(struct ipc_server_t *server);
    wpd_error_t (*const recv)(struct ipc_server_t *server,
                              struct ipc_msg_t *msg);
} _IpcServer;

extern _IpcServer const IpcServer;

/* IpcClient Interface */

struct ipc_client_t;

typedef struct {
    wpd_error_t (*const create)(struct ipc_client_t **client);
    wpd_error_t (*const destroy)(struct ipc_client_t **client);
    wpd_error_t (*const connect)(struct ipc_client_t *client);
    wpd_error_t (*const disconnect)(struct ipc_client_t *client);
    wpd_error_t (*const send)(struct ipc_client_t *client, size_t byte_count,
                              void *bytes);
    wpd_error_t (*const recv)(struct ipc_client_t *client,
                              struct ipc_msg_t *msg);
} _IpcClient;

extern _IpcClient const IpcClient;

#endif // IPC_H
