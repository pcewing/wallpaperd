#include "log.h"
#include "util.h"
#include "enumerate.h"

void
print_debug_info(const struct file_enumeration_node_t* node)
{
    if (node->m_path) LOGINFO("Processing path: %s", node->m_path);
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

    iterate_file_enumeration(enumeration, print_debug_info);
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

