#include "core.h"
#include "log.h"
#include "config.h"
#include "wallpaper.h"
#include "data.h"
#include "ftw.h"

wpd_error_t
wpd_main_loop(const char *search_path)
{
    // Seed the rng that will be used to select images
    wpd_srand();

    struct wpd_db_t *db;
    wpd_error_t error = initialize_database(&db);
    if (error != WPD_ERROR_SUCCESS)
        return error;

    error = wpd_ftw(db, search_path);
    if (error != WPD_ERROR_SUCCESS)
        return error;

    while (1)
    {
        error = wpd_set_wallpapers(db);
        if (error != WPD_ERROR_SUCCESS)
            return error;

        if (wpd_sleep(2) != 0)
        {
            LOGERROR("interrupted by a signal handler");
            wpd_exit(-1);
        }
    }

    error = cleanup_database(&db);
    if (error)
    {
        return error;
    }

    return WPD_ERROR_SUCCESS;
}

int
main(int argc, char *argv[])
{
    struct wpd_config_t* config;
    wpd_error_t error = load_config(&config);
    if (error != WPD_ERROR_SUCCESS)
    {
        LOGERROR("Failed to load config file: %s\n", wpd_error_str(error));
        wpd_exit(error);
    }
    
    // TODO: Switch this back
    //search_path = (argc > 1) ? argv[1] : ".";
    char *search_path = (argc > 1) ? argv[1] : "/home/pewing/box/pic/Wallpapers";

    error = wpd_main_loop(search_path);
    if (error != WPD_ERROR_SUCCESS)
    {
        LOGERROR("Unhandled error encountered: %s\n", wpd_error_str(error));
        wpd_exit(error);
    }

    wpd_exit(WPD_ERROR_SUCCESS);
}

