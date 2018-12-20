#include "core.h"
#include "log.h"
#include "enumerate.h"
#include "wallpaper.h"

wpd_error_t
wpd_main_loop(const char *search_path)
{
    // Seed the rng that will be used to select images
    wpd_srand();

    while (1)
    {
        struct file_enumeration_t* file_enumeration = NULL;

        TRY(create_file_enumeration(search_path, &file_enumeration));

        wpd_set_wallpapers(file_enumeration);

        destroy_file_enumeration(&file_enumeration);

        if (wpd_sleep(2) != 0)
        {
            LOGERROR("interrupted by a signal handler");
            wpd_exit(-1);
        }
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
    
    search_path = (argc > 1) ? argv[1] : ".";

    error = wpd_main_loop(search_path);
    if (error != WPD_ERROR_SUCCESS)
    {
        LOGERROR("Unhandled error encountered: %s\n", wpd_error_str(error));
        wpd_exit(error);
    }

    wpd_exit(WPD_ERROR_SUCCESS);
}

