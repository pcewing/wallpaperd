#include "ipc.h"
#include "log.h"
#include "parse.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#define SV_SOCK_PATH "/tmp/i3bgd"

#define BUF_SIZE 100
#define BACKLOG 5

// TODO: Replace all WPD_ERROR_GLOBAL_FAILURE with real errors

wpd_error_t open_socket(int *result) {
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

wpd_error_t poll_socket(int sfd) {
    int cfd;

    cfd = accept(sfd, NULL, NULL);
    if (cfd == -1 && errno != EWOULDBLOCK)
        return WPD_ERROR_GLOBAL_FAILURE;

    recv_message_socket(cfd);

    if (close(cfd) == -1)
        return WPD_ERROR_GLOBAL_FAILURE;

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t close_socket(int sfd) {
    if (close(sfd) == -1)
        return WPD_ERROR_GLOBAL_FAILURE;

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t connect_socket(int *result) {
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

wpd_error_t send_message_socket(int sfd, const char *msg) {
    /*
     * TODO: Send header first
     * 
    uint8_t header[4];
    header[0] = (head.length >>  0) & 0x000000FF;
    header[1] = (head.length >>  8) & 0x000000FF;
    header[2] = (head.length >> 16) & 0x000000FF;
    header[3] = (head.length >> 24) & 0x000000FF;

    if (write(sfd, header, 4) != 4)
        return WPD_ERROR_GLOBAL_FAILURE;

    */

    // Send body
    ssize_t len = strlen(msg);
    if (write(sfd, msg, len) != len)
        return WPD_ERROR_GLOBAL_FAILURE;

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t recv_message_socket(int sfd) {
    /*
     * TODO: Receive header first
     *
    void* header[4];
    if (read(sfd, header, 4) != 4) {
        error = WPD_ERROR_GLOBAL_FAILURE;
        goto exit;
    }

    uint32_t length;
    error = parse_uint32(header, &length);
    if (error != WPD_ERROR_GLOBAL_SUCCESS) {
        goto exit;
    }

    printf("Message length: %u", length);

    */

    // Read body
    char buf[BUF_SIZE];
    ssize_t numRead;
    while ((numRead = read(sfd, buf, BUF_SIZE)) > 0)
        LOGINFO("Received IPC message: %s", buf);

    if (numRead == -1)
        return WPD_ERROR_GLOBAL_FAILURE;

    return WPD_ERROR_GLOBAL_SUCCESS;
}

