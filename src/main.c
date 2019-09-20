#include "config.h"
#include "core.h"
#include "data.h"
#include "error.g.h"
#include "ftw.h"
#include "log.h"
#include "signal.h"
#include "wallpaper.h"

#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

// TODO: This isn't thread safe.
static bool running = 1;

struct worker_params_t {
    struct wpd_config_t *config;
};

struct worker_result_t {
    wpd_error_t error;
};

void *worker(void *arg) {
    struct worker_params_t params = *((struct worker_params_t*)arg);
    struct wpd_config_t *config = params.config;
    struct worker_result_t *result;

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

    while (config->rotation.enabled && running) {
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
    result = malloc(sizeof(struct worker_result_t));
    assert(result);
    result->error = error;
    return (void *)result;
}

bool handle_input() {
    char *line = NULL;
    size_t len = 0;
    bool exit = false;

    ssize_t nread = getline(&line, &len, stdin);

    // TODO: This is gross; instead of replacing the trailing newline with a
    // null, we should trim leading/trailing whitespace.
    line[nread - 1] = '\0';

    if (nread > 0) {
        printf("Retrieved line of length %zu:\n", nread);
        if (strcmp(line, "quit") == 0) {
            exit = true;
        }
    }

    // The man page for getline states to free this even if getline fails
    free(line);

    return exit;
}

int main(int argc, char *argv[]) {
    // TODO: Make argument parsing more robust
    bool interactive = false;
    for (int i = 1; i < argc; ++i) {
        if (strcmp("-i", argv[i]) || strcmp("--interactive", argv[i])) {
            LOGINFO("Interactive mode enabled; logs will be suppressed!");
            interactive = true;
            suppress_logs();
        }
    }

    struct wpd_config_t *config;
    wpd_error_t error = load_config(&config);
    if (error != WPD_ERROR_GLOBAL_SUCCESS) {
        LOGERROR("Failed to load config file: %s", wpd_error_str(error));
        wpd_exit(error);
    }

    struct worker_params_t worker_params = { .config = config };

    pthread_t thread;
    int s = pthread_create(&thread, NULL, worker, &worker_params);
    assert(s == 0);

    // TODO: Major bug here; if config.rotation.enabled = false, this shouldn't
    // loop forever
    while (1) {
        if (interactive ) {
            if (handle_input()) {
                running = false;
                break;
            }
        } else {
            // For now, if the application isn't interactive the main thread
            // will just sit around until a SIGINT signal is received. In the
            // future, perhaps it could listen for commands on a socket.
            wpd_usleep(33333);
        }
    }

    struct worker_result_t *worker_result;
    pthread_join(thread, (void **)&worker_result);
    assert(worker_result);
    free(worker_result);

    destroy_config(&config);

    wpd_exit(WPD_ERROR_GLOBAL_SUCCESS);
}
