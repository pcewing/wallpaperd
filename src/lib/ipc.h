#ifndef IPC_H
#define IPC_H

#include "core.h"

struct ipc_message_head_t {
    uint32_t length;
};

struct ipc_message_body_t {
    const void *data;
};

struct ipc_message_t {
    struct ipc_message_head_t *head;
    struct ipc_message_body_t *body;
};

// Server Interface

wpd_error_t open_socket(int *result);
wpd_error_t poll_socket(int sfd);
wpd_error_t close_socket(int sfd);

// Client Interface

wpd_error_t connect_socket(int *result);

// Common Interface

wpd_error_t send_message_socket(int sfd, const char *msg);
wpd_error_t recv_message_socket(int sfd);

#endif // IPC_H
