#include "config.h"
#include "core.h"
#include "data.h"
#include "error.g.h"
#include "ftw.h"
#include "log.h"
#include "signal.h"
#include "wallpaper.h"

wpd_error_t wpd_main_loop(struct wpd_config_t *config) {
    // Seed the rng that will be used to select images
    wpd_srand();

    struct wpd_db_t *db;
    wpd_error_t error = initialize_database(&db);
    if (error != WPD_ERROR_GLOBAL_SUCCESS)
        return error;

    for (uint32_t i = 0; i < config->search_path_count; ++i) {
        error = wpd_ftw(db, config->search_paths[i]);
        if (error != WPD_ERROR_GLOBAL_SUCCESS)
            return error;
    }

    error = wpd_set_wallpapers(db);
    if (error != WPD_ERROR_GLOBAL_SUCCESS)
        return error;

    while (config->rotation.enabled) {
        wpd_sleep(config->rotation.frequency);

        error = wpd_set_wallpapers(db);
        if (error != WPD_ERROR_GLOBAL_SUCCESS)
            return error;
    }

    error = cleanup_database(&db);
    if (error) {
        return error;
    }

    return WPD_ERROR_GLOBAL_SUCCESS;
}

int main(int argc, char *argv[]) {
    UNUSED(argc);
    UNUSED(argv);

    wpd_error_t error;

    struct wpd_config_t *config;
    error = load_config(&config);
    if (error != WPD_ERROR_GLOBAL_SUCCESS) {
        LOGERROR("Failed to load config file: %s", wpd_error_str(error));
        wpd_exit(error);
    }

    error = wpd_main_loop(config);
    if (error != WPD_ERROR_GLOBAL_SUCCESS) {
        LOGERROR("Unhandled error encountered: %s", wpd_error_str(error));
        wpd_exit(error);
    }

    destroy_config(&config);

    wpd_exit(WPD_ERROR_GLOBAL_SUCCESS);
}
