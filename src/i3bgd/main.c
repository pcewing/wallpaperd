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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static bool paused = false;

wpd_error_t pause_request_handler(const void *req, void *res) {
    assert(req);
    assert(res);

    struct pause_request_t *pause_request = (struct pause_request_t *)req;
    struct pause_response_t *pause_response = (struct pause_response_t *)res;

    LOGINFO("Received pause request with duration = %ums",
            pause_request->duration_ms);

    paused = !paused;
    pause_response->success = true;

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t discover_request_handler(const void *req, void *res) {
    UNUSED(req);
    assert(res);

    struct discover_response_t *discover_response =
        (struct discover_response_t *)res;

    LOGINFO("Received discover request");
    discover_response->pid = (int32_t)getpid();

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t wpd_main_loop(struct wpd_config_t *config,
                          struct rpc_server_t *rpc_server) {
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
        error = RpcServer.poll(rpc_server);
        if (error != WPD_ERROR_GLOBAL_SUCCESS &&
            error != WPD_ERROR_RPC_INVALID_MESSAGE) {
            LOGERROR("Failed to poll RPC server: %s", wpd_error_str(error));
        }

        if (!paused) {
            uint64_t now = get_timestamp_us();
            if ((now - last_update) > (1000000 * config->rotation.frequency)) {
                error = wpd_set_wallpapers(db);
                if (error != WPD_ERROR_GLOBAL_SUCCESS)
                    goto exit;
                last_update = now;
            }
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

bool is_running() {
    wpd_error_t error;
    bool running = false;

    struct discover_request_t request;
    struct discover_response_t *response =
        malloc(sizeof(struct discover_response_t));

    error = RpcClient.discover(&request, &response);
    if (error == WPD_ERROR_GLOBAL_SUCCESS) {
        running = getpid() != response->pid;
    } else if (error != WPD_ERROR_IPC_FAILED_TO_CONNECT) {
        LOGERROR("Service discovery failed: %s", wpd_error_str(error));
    }

    free(response);
    return running;
}

// TODO: Remove this before mergin RPC changes into master
//#define RPC_TEST_MAIN

int main(int argc, char *argv[]) {
    UNUSED(argc);
    UNUSED(argv);

#ifdef RPC_TEST_MAIN

    // First discover any existing servers because opening the domain socket
    // will hose them
    if (is_running()) {
        LOGERROR("Server is already running");
        wpd_exit(WPD_ERROR_GLOBAL_ALREADY_RUNNING);
    }

    struct rpc_server_t *rpc_server = NULL;
    RpcServer.create(&rpc_server);
    RpcServer.register_handler(rpc_server, RPC_TYPE_PAUSE,
                               pause_request_handler);
    RpcServer.register_handler(rpc_server, RPC_TYPE_DISCOVER,
                               discover_request_handler);

    RpcServer.start(rpc_server);

    while (1)
        RpcServer.poll(rpc_server);

    RpcServer.stop(rpc_server);

    RpcServer.destroy(&rpc_server);

#else

    if (is_running()) {
        LOGERROR("Server is already running");
        wpd_exit(WPD_ERROR_GLOBAL_ALREADY_RUNNING);
    }

    wpd_error_t error;

    struct rpc_server_t *rpc_server = NULL;
    error = RpcServer.create(&rpc_server);
    if (error != WPD_ERROR_GLOBAL_SUCCESS) {
        LOGERROR("Failed to create RPC server: %s", wpd_error_str(error));
        wpd_exit(error);
    }

    RpcServer.register_handler(rpc_server, RPC_TYPE_PAUSE,
                               pause_request_handler);
    RpcServer.register_handler(rpc_server, RPC_TYPE_DISCOVER,
                               discover_request_handler);

    error = RpcServer.start(rpc_server);
    if (error != WPD_ERROR_GLOBAL_SUCCESS) {
        LOGERROR("Failed to start RPC server: %s", wpd_error_str(error));
        wpd_exit(error);
    }

    struct wpd_config_t *config;
    error = load_config(&config);
    if (error != WPD_ERROR_GLOBAL_SUCCESS) {
        LOGERROR("Failed to load config file: %s", wpd_error_str(error));
        wpd_exit(error);
    }

    error = wpd_main_loop(config, rpc_server);
    if (error != WPD_ERROR_GLOBAL_SUCCESS) {
        LOGERROR("Unhandled error encountered: %s", wpd_error_str(error));
        wpd_exit(error);
    }

    destroy_config(&config);

    error = RpcServer.stop(rpc_server);
    if (error != WPD_ERROR_GLOBAL_SUCCESS) {
        LOGERROR("Failed to stop RPC server: %s", wpd_error_str(error));
        wpd_exit(error);
    }

    error = RpcServer.destroy(&rpc_server);
    if (error != WPD_ERROR_GLOBAL_SUCCESS) {
        LOGERROR("Failed to destroy RPC server: %s", wpd_error_str(error));
        wpd_exit(error);
    }

    wpd_exit(WPD_ERROR_GLOBAL_SUCCESS);
#endif
}
