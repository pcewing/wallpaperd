#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <sqlite3.h>

#include "data.h"
#include "log.h"

struct wpd_db_t {
    sqlite3 *handle;
};

//
// Forward declare private functions
//

wpd_error_t create_tables(sqlite3 *db);
int create_wallpaper_table(sqlite3 *db);

//
// Public Interface
//

wpd_error_t initialize_database(struct wpd_db_t **result) {
    struct wpd_db_t *db = malloc(sizeof(struct wpd_db_t));

    int rc = sqlite3_open(":memory:", &db->handle);
    if (rc != 0) {
        LOGERROR("Can't open database: %s", sqlite3_errmsg(db->handle));
        sqlite3_close(db->handle);
        return WPD_ERROR_DATA_SQLITE_OPEN_FAILURE;
    }

    wpd_error_t error = create_tables(db->handle);
    if (error != WPD_ERROR_GLOBAL_SUCCESS) {
        sqlite3_close(db->handle);
        return error;
    }

    *result = db;
    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t insert_wallpaper(const struct wpd_db_t *db,
                             const struct wpd_wallpaper_entity_t *wallpaper) {
    assert(wallpaper);

    // clang-format off
    const char *query_template =
        "insert into "
        "    wallpaper (path, width, height) "
        "values "
        "    ('%s', '%u', '%u');";
    // clang-format on

    char *query = NULL;
    asprintf(&query, query_template, wallpaper->path, wallpaper->width,
             wallpaper->height);

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db->handle, query, strlen(query), &stmt, NULL);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOGERROR("Failed to insert wallpaper: %s", sqlite3_errmsg(db->handle));
        return 1;
    }

    sqlite3_finalize(stmt);
    free(query);

    return 0;
}

int get_wallpaper_count(const struct wpd_db_t *db) {
    const char *query = "select count(*) from wallpaper;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->handle, query, -1, &stmt, NULL);
    assert(rc == SQLITE_OK);

    rc = sqlite3_step(stmt);
    assert(rc == SQLITE_ROW);

    int count = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return count;
}

int get_wallpaper_by_dimensions_count(const struct wpd_db_t *db, int width,
                                      int height) {
    // clang-format off
    const char *query_template =
        "select "
        "    count(*) "
        "from "
        "   wallpaper "
        "where "
            "width = %u and height = %u;";
    // clang-format on

    char *query;
    asprintf(&query, query_template, width, height);

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->handle, query, -1, &stmt, NULL);
    assert(rc == SQLITE_OK);

    rc = sqlite3_step(stmt);
    assert(rc == SQLITE_ROW);

    int count = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    free(query);

    return count;
}

wpd_error_t get_wallpapers(const struct wpd_db_t *db,
                           struct wpd_wallpaper_result_set_t **result_set) {
    int wallpaper_count = get_wallpaper_count(db);

    // TODO: This could be a legitimate error if the configured search paths
    // didn't turn up any wallpapers.
    assert(wallpaper_count > 0);

    struct wpd_wallpaper_entity_t *wallpapers =
        malloc(wallpaper_count * sizeof(struct wpd_wallpaper_entity_t));

    const char *query = "select path, width, height from wallpaper;";

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db->handle, query, strlen(query), &stmt, NULL);

    int i = 0, rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        wallpapers[i].path = strdup((const char *)sqlite3_column_text(stmt, 0));
        wallpapers[i].width = sqlite3_column_int(stmt, 1);
        wallpapers[i].height = sqlite3_column_int(stmt, 2);

        i++;
    }

    sqlite3_finalize(stmt);

    (*result_set) = malloc(sizeof(struct wpd_wallpaper_result_set_t));
    (*result_set)->results = wallpapers;
    (*result_set)->count = wallpaper_count;

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t
get_wallpapers_by_dimensions(const struct wpd_db_t *db, int width, int height,
                             struct wpd_wallpaper_result_set_t **result_set) {
    int wallpaper_count = get_wallpaper_by_dimensions_count(db, width, height);

    // TODO: This could be a legitimate error if the configured search paths
    // didn't turn up any wallpapers.
    assert(wallpaper_count > 0);

    struct wpd_wallpaper_entity_t *wallpapers =
        malloc(wallpaper_count * sizeof(struct wpd_wallpaper_entity_t));

    // clang-format off
    const char *query_template =
        "select "
        "    path, width, height "
        "from "
        "    wallpaper "
        "where "
        "    width = %u and height = %u;";
    // clang-format on

    char *query;
    asprintf(&query, query_template, width, height);

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db->handle, query, strlen(query), &stmt, NULL);

    int i, rc;
    for (i = 0; (rc = sqlite3_step(stmt)) == SQLITE_ROW; i++) {
        wallpapers[i].path = strdup((const char *)sqlite3_column_text(stmt, 0));
        wallpapers[i].width = sqlite3_column_int(stmt, 1);
        wallpapers[i].height = sqlite3_column_int(stmt, 2);
    }

    sqlite3_finalize(stmt);
    free(query);

    (*result_set) = malloc(sizeof(struct wpd_wallpaper_result_set_t));
    (*result_set)->results = wallpapers;
    (*result_set)->count = wallpaper_count;

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t cleanup_database(struct wpd_db_t **db) {
    assert(db);
    assert(*db);

    sqlite3_close((*db)->handle);
    free(*db);

    return WPD_ERROR_GLOBAL_SUCCESS;
}

//
// Private Helper Functions
//

wpd_error_t create_tables(sqlite3 *db) {
    int rc = create_wallpaper_table(db);
    if (rc) {
        sqlite3_close(db);
        return WPD_ERROR_DATA_TABLE_CREATION_FAILURE;
    }

    return WPD_ERROR_GLOBAL_SUCCESS;
}

int create_wallpaper_table(sqlite3 *db) {
    // clang-format off
    const char *query = 
        "create table wallpaper ("
            "wallpaper_id integer primary key,"
            "width integer not null,"
            "height integer not null,"
            "path text not null unique"
        ");";
    // clang-format on

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, query, -1, &stmt, NULL);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOGERROR("Failed to create wallpaper database table: %s",
                 sqlite3_errmsg(db));
        return rc;
    }

    sqlite3_finalize(stmt);

    return 0;
}
