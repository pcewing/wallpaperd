#include "config.h"
#include "core.h"
#include "data.h"
#include "error.g.h"
#include "ftw.h"
#include "log.h"
#include "rpc.h"
#include "signal.h"
#include "wallpaper.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void wpd_quit() { wpd_exit(0); }

void wpd_pause() {
    struct pause_request_t request = {.duration_ms = 0};

    struct pause_response_t *response = malloc(sizeof(struct pause_response_t));
    wpd_error_t error = RpcClient.pause(&request, &response);
    if (error != WPD_ERROR_GLOBAL_SUCCESS) {
        printf("Failed to execute the pause RPC\n");
        free(response);
        return;
    }

    if (response->success)
        printf("Server successfully paused!\n");
    else
        printf("Server failed to pause!\n");

    free(response);
}

void wpd_discover() {
    struct discover_request_t request;

    struct discover_response_t *response =
        malloc(sizeof(struct discover_response_t));
    wpd_error_t error = RpcClient.discover(&request, &response);
    if (error != WPD_ERROR_GLOBAL_SUCCESS) {
        printf("Failed to execute the discover RPC\n");
        free(response);
        return;
    }

    printf("i3bgd daemon running with pid %u!\n", response->pid);

    free(response);
}

#define WPD_COMMAND_TYPE_QUIT 0
#define WPD_COMMAND_TYPE_PAUSE 1
#define WPD_COMMAND_TYPE_DISCOVER 2

typedef uint32_t wpd_command_type_t;
typedef void (*command_func)(void);

struct wpd_command_t {
    wpd_command_type_t type;
    const char **verbs;
    command_func func;
};

#define STRARR(strings...)                                                     \
    (const char *[]) { strings, NULL }

// clang-format off
struct wpd_command_t commands[] = {
    [WPD_COMMAND_TYPE_QUIT] = {
        .type = WPD_COMMAND_TYPE_QUIT,
        .verbs = STRARR("quit", "q", "exit"),
        .func = wpd_quit
    },
    [WPD_COMMAND_TYPE_PAUSE] = {
        .type = WPD_COMMAND_TYPE_PAUSE,
        .verbs = STRARR("pause", "p"),
        .func = wpd_pause
    },
    [WPD_COMMAND_TYPE_DISCOVER] = {
        .type = WPD_COMMAND_TYPE_DISCOVER,
        .verbs = STRARR("discover", "d"),
        .func = wpd_discover
    }
};
// clang-format on

// TODO: this is copy pasted from parse.c
struct wpd_command_t *get_command_by_verb(const char *verb) {
    struct wpd_command_t *result = NULL;
    char *v = wpd_strdup_lower(verb);

    for (uint32_t i = 0; i < ARRLEN_U32(commands); ++i) {
        if (strmatch(commands[i].verbs, v)) {
            result = &commands[i];
            break;
        }
    }

    free(v);
    return result;
}

bool handle_input() {
    char *line = NULL;
    size_t len = 0;
    bool exit = false;

    printf("wpd> ");
    ssize_t nread = getline(&line, &len, stdin);

    // TODO: This is gross; instead of replacing the trailing newline with a
    // null, we should trim leading/trailing whitespace.
    line[nread - 1] = '\0';

    if (nread > 0) {
        struct wpd_command_t *command = get_command_by_verb(line);
        if (!command)
            printf("Unknown command %s\n", line);
        else
            command->func();
    }

    // The man page for getline states to free this even if getline fails
    free(line);

    return exit;
}

int main(int argc, char *argv[]) {
    UNUSED(argc);
    UNUSED(argv);

    while (1)
        if (handle_input())
            break;

    return WPD_ERROR_GLOBAL_SUCCESS;
}
