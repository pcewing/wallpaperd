#include "ipc.h"
#include "log.h"
#include "parse.h"
#include "rpc.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SV_SOCK_PATH "/tmp/i3bgd"

#define BACKLOG 5

struct ipc_msg_head_t {
    uint32_t length;
    uint8_t pad[28]; // Pad the entire header packet to 32 bytes
};

#define HEADER_LENGTH sizeof(struct ipc_msg_head_t)
_Static_assert(HEADER_LENGTH == 32, "IPC message head must be 32 bytes");

struct ipc_msg_t {
    struct ipc_msg_head_t head;
    uint8_t *body;
};

struct ipc_server_t {
    int ssfd; // Server socket file descriptor
    int csfd; // Client socket file descriptor
};

struct ipc_client_t {
    int csfd; // Client socket file descriptor
};

//#define OUTPUT_RAW_TRAFFIC

#ifdef OUTPUT_RAW_TRAFFIC
void print_raw_data(const char *prefix, uint32_t length, uint8_t *bytes) {
    printf("%s< ", prefix);
    if (length > 0)
        printf("%i", bytes[0]);

    for (uint32_t i = 1; i < length; ++i)
        printf(", %i", bytes[i]);

    printf(" >\n");
}
#endif

wpd_error_t ipc_start(int *result) {
    int sfd;
    struct sockaddr_un addr;

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
        return WPD_ERROR_IPC_FAILED_TO_CREATE_SOCKET;

    int flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1)
        return WPD_ERROR_IPC_FAILED_TO_GET_FLAGS;

    flags |= O_NONBLOCK;

    if (fcntl(sfd, F_SETFL, flags) == -1)
        return WPD_ERROR_IPC_FAILED_TO_SET_FLAGS;

    if (strlen(SV_SOCK_PATH) > sizeof(addr.sun_path) - 1)
        return WPD_ERROR_IPC_INVALID_ADDRESS_PATH_LENGTH;

    if (remove(SV_SOCK_PATH) == -1 && errno != ENOENT)
        return WPD_ERROR_IPC_FAILED_TO_REMOVE_SOCKET;

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (bind(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1)
        return WPD_ERROR_IPC_FAILED_TO_BIND_SOCKET;

    if (listen(sfd, BACKLOG) == -1)
        return WPD_ERROR_IPC_FAILED_TO_LISTEN_ON_SOCKET;

    *result = sfd;
    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t ipc_send_head(int sfd, struct ipc_msg_head_t *head) {
    // Serialize head
    uint8_t head_bytes[HEADER_LENGTH];
    memset(head_bytes, 0, HEADER_LENGTH);

    head_bytes[0] = (head->length & 0xFF);
    head_bytes[1] = (head->length >> 8) & 0xFF;
    head_bytes[2] = (head->length >> 16) & 0xFF;
    head_bytes[3] = (head->length >> 24) & 0xFF;

    // Send head
    if (write(sfd, head_bytes, HEADER_LENGTH) != HEADER_LENGTH)
        return WPD_ERROR_IPC_FAILED_TO_WRITE_TO_SOCKET;

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t ipc_send_body(int sfd, struct ipc_msg_t *msg) {
    assert(msg);

    ssize_t bytes_written = write(sfd, msg->body, msg->head.length);
    if (bytes_written == -1) {
        LOGERROR("Failed to write to IPC socket");
        return WPD_ERROR_IPC_FAILED_TO_WRITE_TO_SOCKET;
    } else if ((size_t)bytes_written != msg->head.length) {
        LOGERROR("Unexpected number of bytes written to IPC socket");
        return WPD_ERROR_IPC_UNEXPECTED_WRITE_LENGTH;
    }

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t ipc_send(int sfd, size_t byte_count, void *bytes) {
    assert(sfd >= 0);

    struct ipc_msg_t msg = {.head = {.length = byte_count}, .body = bytes};

    // Send header
    wpd_error_t error = ipc_send_head(sfd, &msg.head);
    if (error != WPD_ERROR_GLOBAL_SUCCESS) {
        return error;
    }

    // Send body
    error = ipc_send_body(sfd, &msg);
    if (error != WPD_ERROR_GLOBAL_SUCCESS) {
        return error;
    }

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t recv_head(int sfd, struct ipc_msg_head_t *head) {
    uint8_t head_bytes[HEADER_LENGTH];
    if (read(sfd, head_bytes, HEADER_LENGTH) != HEADER_LENGTH)
        return WPD_ERROR_IPC_FAILED_TO_READ_FROM_SOCKET;

    uint32_t length = head_bytes[0];
    length |= head_bytes[1] << 8;
    length |= head_bytes[2] << 16;
    length |= head_bytes[3] << 24;

#ifdef OUTPUT_RAW_TRAFFIC
    print_raw_data("Head: ", HEADER_LENGTH, header_bytes);
#endif

    head->length = length;
    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t recv_body(int sfd, uint32_t length, uint8_t **body) {
    uint8_t *buf = malloc(length * sizeof(uint8_t));
    ssize_t num_read = read(sfd, buf, length);
    if (num_read != length) {
        if (num_read == -1)
            LOGERROR("Failed to read from IPC socket");
        else
            LOGERROR("Received IPC message with unexpected length; expected "
                     "length = %u, actual length = %u",
                     length, num_read);

        free(buf);
        return WPD_ERROR_IPC_UNEXPECTED_READ_LENGTH;
    }

#ifdef OUTPUT_RAW_TRAFFIC
    print_raw_data("Body: ", length, buf);
#endif

    *body = buf;
    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t ipc_recv(int sfd, struct ipc_msg_t *msg) {
    assert(msg);

    // Read head
    wpd_error_t error = recv_head(sfd, &msg->head);
    if (error != WPD_ERROR_GLOBAL_SUCCESS)
        return error;

    // Read body
    error = recv_body(sfd, msg->head.length, &msg->body);
    if (error != WPD_ERROR_GLOBAL_SUCCESS)
        return error;

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t ipc_poll(int ssfd, int *csfd, struct ipc_msg_t *msg) {
    *csfd = -1;

    int cfd = accept(ssfd, NULL, NULL);
    if (cfd == -1)
        return (errno != EWOULDBLOCK)
                   ? WPD_ERROR_IPC_FAILED_TO_ACCEPT_CONNECTION
                   : WPD_ERROR_GLOBAL_SUCCESS;

    wpd_error_t error = ipc_recv(cfd, msg);
    if (error != WPD_ERROR_GLOBAL_SUCCESS)
        return error;

    *csfd = cfd;
    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t ipc_close(int *sfd) {
    assert(sfd);

    if (close(*sfd) == -1)
        return WPD_ERROR_IPC_FAILED_TO_CLOSE_SOCKET;

    *sfd = -1;
    return WPD_ERROR_GLOBAL_SUCCESS;
}

void ipc_address(struct sockaddr_un *addr) {
    memset(addr, 0, sizeof(struct sockaddr_un));
    addr->sun_family = AF_UNIX;
    strncpy(addr->sun_path, SV_SOCK_PATH, sizeof(addr->sun_path) - 1);
}

wpd_error_t ipc_connect(int *result) {
    int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
        return WPD_ERROR_IPC_FAILED_TO_CREATE_SOCKET;

    struct sockaddr_un addr_un;
    ipc_address(&addr_un);
    struct sockaddr *addr = (struct sockaddr *)&addr_un;

    if (connect(sfd, addr, sizeof(struct sockaddr_un)) == -1)
        return WPD_ERROR_IPC_FAILED_TO_CONNECT;

    *result = sfd;
    return WPD_ERROR_GLOBAL_SUCCESS;
}

/* IpcMessage */

wpd_error_t ipc_msg_create(struct ipc_msg_t **msg) {
    assert(msg);
    assert(!(*msg));

    *msg = malloc(sizeof(struct ipc_msg_t));
    memset(*msg, 0, sizeof(struct ipc_msg_t));

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t ipc_msg_destroy(struct ipc_msg_t **msg) {
    assert(msg);
    assert(*msg);

    if ((*msg)->body)
        free((*msg)->body);

    free(*msg);
    *msg = NULL;

    return WPD_ERROR_GLOBAL_SUCCESS;
}

uint32_t ipc_msg_size(struct ipc_msg_t *msg) {
    assert(msg);
    return msg->head.length;
}

uint8_t *ipc_msg_data(struct ipc_msg_t *msg) {
    assert(msg);
    return msg->body;
}

/* IpcServer */

wpd_error_t ipc_server_create(struct ipc_server_t **server) {
    assert(server);
    assert(!(*server));

    *server = malloc(sizeof(struct ipc_server_t));
    memset(*server, 0, sizeof(struct ipc_server_t));

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t ipc_server_destroy(struct ipc_server_t **server) {
    assert(server);
    assert(*server);

    free(*server);
    *server = NULL;

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t ipc_server_start(struct ipc_server_t *server) {
    assert(server);
    return ipc_start(&server->ssfd);
}

wpd_error_t ipc_server_poll(struct ipc_server_t *server, bool *accepted,
                            struct ipc_msg_t *msg) {
    assert(server);
    assert(msg);

    *accepted = false;
    wpd_error_t error = ipc_poll(server->ssfd, &server->csfd, msg);
    if (error != WPD_ERROR_GLOBAL_SUCCESS)
        return error;

    *accepted = server->csfd != -1;
    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t ipc_server_close(struct ipc_server_t *server) {
    assert(server);
    return ipc_close(&server->csfd);
}

wpd_error_t ipc_server_stop(struct ipc_server_t *server) {
    assert(server);
    return ipc_close(&server->ssfd);
}

wpd_error_t ipc_server_send(struct ipc_server_t *server, size_t byte_count,
                            void *bytes) {
    assert(server);
    assert(byte_count > 0);
    assert(bytes);
    return ipc_send(server->csfd, byte_count, bytes);
}

wpd_error_t ipc_server_recv(struct ipc_server_t *server,
                            struct ipc_msg_t *msg) {
    assert(server);
    return ipc_recv(server->ssfd, msg);
}

/* IpcClient */

wpd_error_t ipc_client_create(struct ipc_client_t **client) {
    assert(client);
    assert(!(*client));

    *client = malloc(sizeof(struct ipc_client_t));

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t ipc_client_destroy(struct ipc_client_t **client) {
    assert(client);
    assert(*client);

    free(*client);
    *client = NULL;

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t ipc_client_connect(struct ipc_client_t *client) {
    assert(client);
    return ipc_connect(&client->csfd);
}

wpd_error_t ipc_client_disconnect(struct ipc_client_t *client) {
    assert(client);
    return ipc_close(&client->csfd);
}

wpd_error_t ipc_client_send(struct ipc_client_t *client, size_t byte_count,
                            void *bytes) {
    assert(client);
    assert(byte_count > 0);
    assert(bytes);
    return ipc_send(client->csfd, byte_count, bytes);
}

wpd_error_t ipc_client_recv(struct ipc_client_t *client,
                            struct ipc_msg_t *msg) {
    assert(client);
    return ipc_recv(client->csfd, msg);
}

// clang-format off

_IpcMessage const IpcMessage = {
    .create = ipc_msg_create,
    .destroy = ipc_msg_destroy,
    .size = ipc_msg_size,
    .data = ipc_msg_data
};

_IpcServer const IpcServer = {
    .create = ipc_server_create,
    .destroy = ipc_server_destroy,
    .start = ipc_server_start,
    .stop = ipc_server_stop,
    .poll = ipc_server_poll,
    .close = ipc_server_close,
    .send = ipc_server_send,
    .recv = ipc_server_recv
};

_IpcClient const IpcClient = {
    .create = ipc_client_create,
    .destroy = ipc_client_destroy,
    .connect = ipc_client_connect,
    .disconnect = ipc_client_disconnect,
    .send = ipc_client_send,
    .recv = ipc_client_recv
};

// clang-format on
