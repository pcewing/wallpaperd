#include "core.h"
#include "log.h"
#include "wallpaper.h"
#include "data.h"
#include "ftw.h"

wpd_error_t
wpd_main_loop(const char *search_path)
{
    struct wpd_db_t *db;
    wpd_error_t      error;

    // Seed the rng that will be used to select images
    wpd_srand();

    error = initialize_database(&db);
    if (error)
    {
        return error;
    }

    error = wpd_ftw(db, search_path);
    if (error)
    {
        return error;
    }

    while (1)
    {
        TRY(wpd_set_wallpapers(db));

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
    UNUSED(argc);
    UNUSED(argv);

    wpd_error_t error;
    char *search_path;
    
    // TODO: Switch this back
    //search_path = (argc > 1) ? argv[1] : ".";
    search_path = (argc > 1) ? argv[1] : "/home/pewing/box/pic/Wallpapers";

    error = wpd_main_loop(search_path);
    if (error != WPD_ERROR_SUCCESS)
    {
        LOGERROR("Unhandled error encountered: %s\n", wpd_error_str(error));
        wpd_exit(error);
    }

    wpd_exit(WPD_ERROR_SUCCESS);
}

