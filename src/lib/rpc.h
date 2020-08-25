#ifndef RPC_H
#define RPC_H

#include "core.h"

#define RPC_TYPE_PAUSE 0
#define RPC_TYPE_DISCOVER 1

typedef uint32_t rpc_type_t;

typedef wpd_error_t (*rpc_handler)(const void *request, void *response);

struct pause_request_t {
    uint32_t duration_ms;
};

struct pause_response_t {
    bool success;
};

struct discover_request_t {};

struct discover_response_t {
    int32_t pid;
};

/* RpcServer Interface */

struct rpc_server_t;

typedef struct {
    wpd_error_t (*const create)(struct rpc_server_t **server);
    wpd_error_t (*const destroy)(struct rpc_server_t **server);
    wpd_error_t (*const register_handler)(struct rpc_server_t *server,
                                          rpc_type_t type, rpc_handler handler);
    wpd_error_t (*const start)(struct rpc_server_t *server);
    wpd_error_t (*const poll)(struct rpc_server_t *server);
    wpd_error_t (*const stop)(struct rpc_server_t *server);
} _RpcServer;

extern _RpcServer const RpcServer;

/* RpcClient Interface */

typedef struct {
    wpd_error_t (*const pause)(struct pause_request_t *request,
                               struct pause_response_t **response);
    wpd_error_t (*const discover)(struct discover_request_t *request,
                                  struct discover_response_t **response);
} _RpcClient;

extern _RpcClient const RpcClient;

#endif // RPC_H
