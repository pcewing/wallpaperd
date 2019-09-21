#include "ipc.h"
#include "log.h"
#include "parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#define SV_SOCK_PATH "/tmp/i3bgd"

#define BACKLOG 5

// TODO: Replace all WPD_ERROR_GLOBAL_FAILURE with real errors

wpd_error_t ipc_open(int *result) {
    int sfd;
    struct sockaddr_un addr;

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
        return WPD_ERROR_GLOBAL_FAILURE;

    int flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1)
        return WPD_ERROR_GLOBAL_FAILURE;

    flags |= O_NONBLOCK;

    if (fcntl(sfd, F_SETFL, flags) == -1)
        return WPD_ERROR_GLOBAL_FAILURE;

    if (strlen(SV_SOCK_PATH) > sizeof(addr.sun_path) - 1)
        return WPD_ERROR_GLOBAL_FAILURE;

    if (remove(SV_SOCK_PATH) == -1 && errno != ENOENT)
        return WPD_ERROR_GLOBAL_FAILURE;

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1)
        return WPD_ERROR_GLOBAL_FAILURE;

    if (listen(sfd, BACKLOG) == -1)
        return WPD_ERROR_GLOBAL_FAILURE;

    *result = sfd;
    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t ipc_poll(int sfd, struct ipc_message_t **msg) {
    int cfd;

    cfd = accept(sfd, NULL, NULL);
    if (cfd == -1) {
        *msg = NULL;
        return (errno != EWOULDBLOCK) ? WPD_ERROR_GLOBAL_FAILURE : WPD_ERROR_GLOBAL_SUCCESS;
    }

    *msg = malloc(sizeof(struct ipc_message_t));
    
    if (ipc_recv(cfd, *msg) != WPD_ERROR_GLOBAL_SUCCESS) {
        free(*msg);
        *msg = NULL;
        return WPD_ERROR_GLOBAL_FAILURE;
    }

    // TODO: If this fails, free msg
    if (close(cfd) == -1)
        return WPD_ERROR_GLOBAL_FAILURE;

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t ipc_close(int sfd) {
    if (close(sfd) == -1)
        return WPD_ERROR_GLOBAL_FAILURE;

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t ipc_connect(int *result) {
    struct sockaddr_un addr;
    int sfd;

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
        return WPD_ERROR_GLOBAL_FAILURE;

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1)
        return WPD_ERROR_GLOBAL_FAILURE;

    *result = sfd;
    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t ipc_send(int sfd, const char *msg) {
    // Calculate message length
    ssize_t length = strlen(msg);

    // Initialize header
    uint8_t header[HEADER_LENGTH];
    header[0] = length & 0xFF;
    header[1] = (length >>  8) & 0xFF;
    header[2] = (length >> 16) & 0xFF;
    header[3] = (length >> 24) & 0xFF;

    // Send header
    if (write(sfd, header, HEADER_LENGTH) != HEADER_LENGTH)
        return WPD_ERROR_GLOBAL_FAILURE;

    // Send body
    if (write(sfd, msg, length) != length)
        return WPD_ERROR_GLOBAL_FAILURE;

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t ipc_recv(int sfd, struct ipc_message_t *msg) {
    assert(msg);

    uint8_t header[HEADER_LENGTH];
    if (read(sfd, header, HEADER_LENGTH) != HEADER_LENGTH)
        return WPD_ERROR_GLOBAL_FAILURE;

    uint32_t length = header[0];
    length |= header[1] <<  8;
    length |= header[2] << 16;
    length |= header[3] << 24;

    // Read body
    uint8_t *buf = malloc(length * sizeof(uint8_t));
    ssize_t num_read = read(sfd, buf, length);
    if (num_read == length)
        LOGINFO("Received IPC message of length %u", length);
    else {
        if (num_read == -1)
            LOGERROR("Failed to read from IPC socket");
        else
            LOGERROR("Received IPC message with unexpected length; expected length = %u, actual length = %u", length, num_read);

        free(buf);
        return WPD_ERROR_GLOBAL_FAILURE;
    }

    // TODO: buf needs to be freed by the caller
    msg->head.length = length;
    msg->body.data = buf;
    return WPD_ERROR_GLOBAL_SUCCESS;
}

