#ifndef IPC_H
#define IPC_H

#include "core.h"

struct ipc_message_head_t {
    uint32_t length;
    uint8_t pad[28]; // Pad the entire header packet to 32 bytes
};

#define HEADER_LENGTH sizeof(struct ipc_message_head_t)

struct ipc_message_body_t {
    uint8_t *data;
};

struct ipc_message_t {
    struct ipc_message_head_t head;
    struct ipc_message_body_t body;
};

// Server Interface

wpd_error_t ipc_open(int *result);
wpd_error_t ipc_poll(int sfd, struct ipc_message_t **msg);

// Client Interface

wpd_error_t ipc_connect(int *result);

// Common Interface

wpd_error_t ipc_send(int sfd, const char *msg);
wpd_error_t ipc_recv(int sfd, struct ipc_message_t *msg);
wpd_error_t ipc_close(int sfd);

#endif // IPC_H
