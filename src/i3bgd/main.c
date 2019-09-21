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

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>


static int ipc_socket;

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

static StsHeader *command_queue = NULL;

struct wpd_command_definition_t *get_command_definition_by_verb(const char *verb) {
    struct wpd_command_definition_t *result = NULL;
    char *v = wpd_strdup_lower(verb);

    for (uint32_t i = 0; i < ARRLEN_U32(command_definitions); ++i) {
        if (strmatch(command_definitions[i].verbs, v)) {
            result = &command_definitions[i];
            break;
        }
    }

    free(v);
    return result;
}

void print_raw_data(struct ipc_message_t *msg) {
    if (msg->head.length > 0)
        printf("< %i", msg->body.data[0]);

    for (uint32_t i = 1; i < msg->head.length; ++i) {
        printf(", %i", msg->body.data[i]);
    }

    printf(" >\n");
}

wpd_error_t wpd_main_loop(struct wpd_config_t *config) {
    // Seed the rng that will be used to select images
    wpd_srand();

    struct wpd_db_t *db;
    wpd_error_t error = initialize_database(&db);
    if (error != WPD_ERROR_GLOBAL_SUCCESS)
        goto exit;

    for (uint32_t i = 0; i < config->search_path_count; ++i) {
        error = wpd_ftw(db, config->search_paths[i]);
        if (error != WPD_ERROR_GLOBAL_SUCCESS)
            goto exit;
    }

    error = wpd_set_wallpapers(db);
    if (error != WPD_ERROR_GLOBAL_SUCCESS)
        goto exit;
    uint64_t last_update = get_timestamp_us();

    while (config->rotation.enabled) {
        struct ipc_message_t *msg;
        error = ipc_poll(ipc_socket, &msg);
        if (error == WPD_ERROR_GLOBAL_SUCCESS && msg) {
            print_raw_data(msg);
            free(msg->body.data);
            free(msg);
        }

        struct wpd_command_t *command;
        while ( (command = StsQueue.pop(command_queue)) != NULL) {
            command_definitions[command->type].func();
            free(command);
        }

        uint64_t now = get_timestamp_us();
        if ((now - last_update) > (1000000 * config->rotation.frequency)) {
            error = wpd_set_wallpapers(db);
            if (error != WPD_ERROR_GLOBAL_SUCCESS)
                goto exit;
            last_update = now;
        }

        // 30 FPS
        wpd_usleep(33333);
    }

    error = cleanup_database(&db);
    if (error) {
        goto exit;
    }

exit:
    return error;
}

int main(int argc, char *argv[]) {
    UNUSED(argc);
    UNUSED(argv);

    wpd_error_t error;

    // TODO: This doesn't fail if the socket already exists; it will open and
    // the existing process' socket will be closed
    error = ipc_open(&ipc_socket);
    if (error != WPD_ERROR_GLOBAL_SUCCESS) {
        LOGERROR("Failed to open the IPC socket: %s", wpd_error_str(error));
        wpd_exit(error);
    }

    struct wpd_config_t *config;
    error = load_config(&config);
    if (error != WPD_ERROR_GLOBAL_SUCCESS) {
        LOGERROR("Failed to load config file: %s", wpd_error_str(error));
        wpd_exit(error);
    }

    // TODO: Nothing is populating this command queue yet; the ipc socket
    // poller should be deserializing commands and pushing them onto this queue
    command_queue = StsQueue.create();

    error = wpd_main_loop(config);
    if (error != WPD_ERROR_GLOBAL_SUCCESS) {
        LOGERROR("Unhandled error encountered: %s", wpd_error_str(error));
        wpd_exit(error);
    }

    StsQueue.destroy(command_queue);

    destroy_config(&config);

    ipc_close(ipc_socket);

    wpd_exit(WPD_ERROR_GLOBAL_SUCCESS);
}
