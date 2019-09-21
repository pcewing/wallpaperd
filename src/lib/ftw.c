#include <assert.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wordexp.h>

#include "core.h"
#include "ftw.h"
#include "image.h"
#include "log.h"

const char *type_str(unsigned char type) {
    const char *retval;

    switch (type) {
    case DT_BLK:
        retval = "[BLOCK DEVICE]     ";
        break;
    case DT_CHR:
        retval = "[CHARACTER DEVICE] ";
        break;
    case DT_DIR:
        retval = "[DIRECTORY]        ";
        break;
    case DT_FIFO:
        retval = "[NAMED PIPE]       ";
        break;
    case DT_LNK:
        retval = "[SYMBOLIC LINK]    ";
        break;
    case DT_REG:
        retval = "[REGULAR FILE]     ";
        break;
    case DT_SOCK:
        retval = "[DOMAIN SOCKET]    ";
        break;
    case DT_UNKNOWN:
        retval = "[UNKNOWN]          ";
        break;

    // If we reach this we have big problems
    default:
        assert(0);
    };

    return retval;
}

bool is_extension_supported(const char *extension) {
    unsigned int i;

    // TODO: Make this configurable
    const char *supported_extensions[] = {"jpg", "png"};

    for (i = 0; i < sizeof(supported_extensions) / sizeof(char *); ++i) {
        if (strcmp(extension, supported_extensions[i]) == 0) {
            return true;
        }
    }

    return false;
}

wpd_error_t process_directory(const struct wpd_db_t *db, const char *path,
                              struct dirent *entry) {
    UNUSED(entry);

    // Recursively walk directories
    wpd_ftw(db, path);

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t process_regular_file(const struct wpd_db_t *db, const char *path,
                                 struct dirent *entry) {
    char *extension;
    wpd_error_t error;
    struct wpd_image_metadata_t *image_metadata;

    extension = wpd_get_extension(entry->d_name);
    if (!extension) {
        return WPD_ERROR_FTW_UNKNOWN_EXTENSION;
    }

    if (is_extension_supported(extension)) {
        error = wpd_get_image_metadata(path, &image_metadata);
        assert(!error);

        // Create the DB entity
        struct wpd_wallpaper_entity_t wallpaper = {
            (char *)path, image_metadata->width, image_metadata->height};

        // Insert the entity into the DB
        insert_wallpaper(db, &wallpaper);

        wpd_free_image_metadata(&image_metadata);
    }

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t process_directory_entry(const struct wpd_db_t *db,
                                    const char *parent_dir,
                                    struct dirent *entry) {
    char *full_path;

    // Ignore the current and parent directory entries
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
        return WPD_ERROR_GLOBAL_SUCCESS;
    }

    full_path = wpd_path_join(parent_dir, entry->d_name);

#define WPD_FTW_CASE_NOOP(TYPE)                                                \
    case TYPE:                                                                 \
        break;
#define WPD_FTW_CASE(TYPE, PROCESSOR)                                          \
    case TYPE: {                                                               \
        wpd_error_t error = PROCESSOR(db, full_path, entry);                   \
        if (error != WPD_ERROR_GLOBAL_SUCCESS) {                               \
            return error;                                                      \
        }                                                                      \
        break;                                                                 \
    }

    switch (entry->d_type) {
        // For now, we only process normal files and directories
        WPD_FTW_CASE(DT_DIR, process_directory)
        WPD_FTW_CASE(DT_REG, process_regular_file)

        // Ignore everything else
        WPD_FTW_CASE_NOOP(DT_BLK)
        WPD_FTW_CASE_NOOP(DT_CHR)
        WPD_FTW_CASE_NOOP(DT_FIFO)
        WPD_FTW_CASE_NOOP(DT_LNK)
        WPD_FTW_CASE_NOOP(DT_SOCK)
        WPD_FTW_CASE_NOOP(DT_UNKNOWN)

    default:
        assert(0);
    };

    free(full_path);

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t wpd_ftw(const struct wpd_db_t *db, const char *path) {
    struct dirent *entry;
    DIR *dirp;
    char *full_path;

    wordexp_t p;
    wordexp(path, &p, 0);

    for (size_t i = 0; i < p.we_wordc; ++i) {
        full_path = malloc(PATH_MAX);
        assert(realpath(p.we_wordv[i], full_path));

        dirp = opendir(full_path);
        assert(dirp);

        while ((entry = readdir(dirp))) {
            wpd_error_t wpd_error =
                process_directory_entry(db, full_path, entry);
            if (wpd_error != WPD_ERROR_GLOBAL_SUCCESS)
                return wpd_error;
        }

        closedir(dirp);
        free(full_path);
    }

    wordfree(&p);

    return WPD_ERROR_GLOBAL_SUCCESS;
}
