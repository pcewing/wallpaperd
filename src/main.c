#include "log.h"
#include "util.h"
#include "enumerate.h"
#include "wallpaper.h"
#include "wallpaper2.h"

#define WALLPAPER
//#define WALLPAPER2

int
main_loop(const struct file_enumeration_t* enumeration)
{
    wpd_srand();

#ifdef WALLPAPER
    static struct x11_context_t* x11 = NULL;
    int error;

    error = create_x11_context(&x11);
    if (error != 0)
    {
        return error;
    }

    initialize_imlib(x11);
#endif

#ifdef WALLPAPER2
    init_x_and_imlib();
#endif

    while (1)
    {
        int index = wpd_rand() % enumeration->node_count;

        LOGINFO("Setting wallpaper to %s", enumeration->nodes[index]->m_path);

#ifdef WALLPAPER
        wpd_set_wallpaper(x11, enumeration->nodes[index]->m_path);
#endif

#ifdef WALLPAPER2
        set_bg(enumeration->nodes[index]->m_path);
#endif

        if (wpd_sleep(2) != 0)
        {
            LOGERROR("interrupted by a signal handler");
            wpd_exit(-1);
        }
    }

#ifdef WALLPAPER
    destroy_x11_context(&x11);
#endif

#ifdef WALLPAPER2
#endif


}

int
process_files(const char * path)
{
    struct file_enumeration_t* enumeration = NULL;

    ftw_error_t enumeration_result = enumerate_files(path, &enumeration);
    if (enumeration_result != FILE_ENUMERATION_SUCCESS)
    {
        return enumeration_result;
    }

    LOGINFO("Found %i wallpaper files!", enumeration->node_count);

    main_loop(enumeration);

    free_enumeration(&enumeration);

    return 0;
}

int
main(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);

    char* path = (argc > 1) ? argv[1] : ".";

    if (process_files(path) != 0)
    {
        LOGERROR("TODO: Uh oh\n");
        wpd_exit(WPD_EXIT_FAILURE);
    }

    wpd_exit(WPD_EXIT_SUCCESS);
}

