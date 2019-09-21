#include "config.h"
#include "core.h"
#include "data.h"
#include "error.g.h"
#include "ftw.h"
#include "log.h"
#include "signal.h"
#include "wallpaper.h"
#include "queue.h"
#include "ipc.h"

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>


wpd_error_t send_message(const char *msg) {
    wpd_error_t error = WPD_ERROR_GLOBAL_SUCCESS;
    int sfd;

    error = ipc_connect(&sfd);
    if (error != WPD_ERROR_GLOBAL_SUCCESS) {
        LOGERROR("Failed to connect to socket");
        goto exit;
    }

    error = ipc_send(sfd, msg);
    if (error != WPD_ERROR_GLOBAL_SUCCESS) {
        LOGERROR("Failed to send ipc message");
        goto exit;
    }

    ipc_close(sfd);

exit:
    return error;
}

// TODO: Put this all in ipc.{h,c} ^^^^^^^^^^^^^^^^^^^^^^^^^


#define ARRLEN(x) (sizeof(x) / sizeof((x)[0]))

#define ARRLEN_U32(x)  (uint32_t) ARRLEN(x)

void quit() {
    printf("Quitting!\n");
    wpd_exit(0);
}

typedef uint32_t wpd_command_type_t;
typedef uint64_t wpd_command_id_t;

#define WPD_COMMAND_TYPE_QUIT 0

typedef void (*command_func)(void);

struct wpd_command_definition_t {
    const char **verbs;
    command_func func;
    wpd_command_type_t type;
};

struct wpd_command_t {
    wpd_command_id_t id;
    wpd_command_type_t type;
};

#define STRARR(strings...) (const char * []) { strings, NULL }

struct wpd_command_definition_t command_definitions[] = {
    [WPD_COMMAND_TYPE_QUIT] = {
        .type = WPD_COMMAND_TYPE_QUIT,
        .verbs = STRARR("quit", "q", "exit"),
        .func = quit
    }
};

// TODO: this is copy pasted from parse.c
bool smatch(const char **haystack, const char *needle) {
    for (int i = 0; haystack[i]; i++)
        if (strcmp(needle, haystack[i]) == 0)
            return true;
    return false;
}

struct wpd_command_definition_t *get_command_definition_by_verb(const char *verb) {
    struct wpd_command_definition_t *result = NULL;
    char *v = wpd_strdup_lower(verb);

    for (uint32_t i = 0; i < ARRLEN_U32(command_definitions); ++i) {
        if (smatch(command_definitions[i].verbs, v)) {
            result = &command_definitions[i];
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
    static int next_id = 0;

    printf("wpd> ");
    ssize_t nread = getline(&line, &len, stdin);

    // TODO: This is gross; instead of replacing the trailing newline with a
    // null, we should trim leading/trailing whitespace.
    line[nread - 1] = '\0';

    if (nread > 0) {
        struct wpd_command_definition_t *command_definition = get_command_definition_by_verb(line);
        if (!command_definition) {
            printf("Unknown command %s\n", line);
        } else {
            struct wpd_command_t command = {
                .type = command_definition->type,
                .id = next_id++
            };

            // TODO: Serialize the command and send it over the socket
            UNUSED(command);
            send_message(command_definition->verbs[0]);
            
            // TODO: Get the response
        }
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
