#include "log.h"
#include "util.h"
#include "enumerate.h"

int
main_loop(const struct file_enumeration_t* enumeration)
{
    // TODO: Actually seed this
    wpd_srand(1);

    while (1)
    {
        int index = wpd_rand() % enumeration->node_count;
        LOGINFO("Setting wallpaper: %s",  enumeration->nodes[index]->m_path);

        if (wpd_sleep(1) != 0)
        {
            LOGERROR("interrupted by a signal handler");
            wpd_exit(-1);
        }
    }
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

