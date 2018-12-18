#include "core.h"
#include "log.h"
#include "enumerate.h"
#include "wallpaper.h"

wpd_error
wpd_initialize(struct x11_context_t** x11)
{
    wpd_srand();

    TRY(wpd_create_x11_context(x11));

    wpd_initialize_libs(*x11);

    return WPD_ERROR_SUCCESS;
}

wpd_error
wpd_cleanup(struct x11_context_t** x11)
{
    TRY(wpd_destroy_x11_context(x11));

    return WPD_ERROR_SUCCESS;
}

wpd_error
wpd_main_loop(const struct file_enumeration_t* enumeration)
{
    static struct x11_context_t* x11 = NULL;

    TRY(wpd_initialize(&x11));

    while (1)
    {
        int index = wpd_rand() % enumeration->node_count;

        LOGINFO("Setting wallpaper to %s", enumeration->nodes[index]->m_path);

        wpd_set_wallpaper(x11, enumeration->nodes[index]->m_path);

        if (wpd_sleep(2) != 0)
        {
            LOGERROR("interrupted by a signal handler");
            wpd_exit(-1);
        }
    }

    TRY(wpd_cleanup(&x11));

    return WPD_ERROR_SUCCESS;
}

wpd_error
wpd_main(const char * path)
{
    struct file_enumeration_t* enumeration = NULL;

    TRY(enumerate_files(path, &enumeration));

    LOGINFO("Found %i wallpaper files!", enumeration->node_count);

    TRY(wpd_main_loop(enumeration));

    free_enumeration(&enumeration);

    return WPD_ERROR_SUCCESS;
}

int
main(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);
    wpd_error error;

    char* path = (argc > 1) ? argv[1] : ".";

    error = wpd_main(path);
    if (error != WPD_ERROR_SUCCESS)
    {
        LOGERROR("Unhandled error encountered: %s\n", wpd_error_str(error));
        wpd_exit(error);
    }

    wpd_exit(WPD_ERROR_SUCCESS);
}

