
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <sqlite3.h>

#include "data.h"

struct wpd_db_t
{
    sqlite3* handle;
};

//
// Forward declare private functions
//

int create_tables(sqlite3 *db);
int create_wallpaper_table(sqlite3 *db);

//
// Public Interface
//

wpd_error_t
initialize_database(struct wpd_db_t** result)
{
    struct wpd_db_t *db;
    int              rc;

    db = malloc(sizeof(struct wpd_db_t));

    rc = sqlite3_open(":memory:", &db->handle);
    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n",
            sqlite3_errmsg(db->handle));
        sqlite3_close(db->handle);
        return WPD_ERROR_TODO;
    }

    rc = create_tables(db->handle);
    if (rc)
    {
        sqlite3_close(db->handle);
        return WPD_ERROR_TODO;
    }

    *result = db;
    return WPD_ERROR_SUCCESS;
}

wpd_error_t
insert_wallpaper(const struct wpd_db_t *db, const struct wpd_wallpaper_entity_t *wallpaper)
{
    char         *query;
    const char   *query_template;
    sqlite3_stmt *stmt;
    int           rc;

    assert(wallpaper);

    query = NULL;
    query_template =
        "insert into "
        "    wallpaper (path, width, height) "
        "values "
        "    ('%s', '%u', '%u');";

    asprintf(&query, query_template, wallpaper->path, wallpaper->width, wallpaper->height);

    sqlite3_prepare_v2(db->handle, query, strlen(query), &stmt, NULL);  

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        printf("Failed to insert wallpaper: %s\n", sqlite3_errmsg(db->handle));
        return 1;
    }

    sqlite3_finalize(stmt); 
    free(query);

    return 0;
}

int
get_wallpaper_count(const struct wpd_db_t *db)
{
    sqlite3_stmt *stmt;
    int           rc,
                  count;
    const char   *query;

    query = "select count(*) from wallpaper;";

    rc = sqlite3_prepare_v2(db->handle, query, -1, &stmt, NULL);
    assert(rc == SQLITE_OK);

    rc = sqlite3_step(stmt);
    assert(rc == SQLITE_ROW);

    count = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt); 

    return count;
}

int
get_wallpaper_by_dimensions_count(const struct wpd_db_t *db, int width, int height)
{
    sqlite3_stmt *stmt;
    int           rc,
                  count;
    const char   *query_template;
    char         *query;

    query_template =
        "select "
        "    count(*) "
        "from "
        "   wallpaper "
        "where "
            "width = %u and height = %u;";

    asprintf(&query, query_template, width, height);

    rc = sqlite3_prepare_v2(db->handle, query, -1, &stmt, NULL);
    assert(rc == SQLITE_OK);

    rc = sqlite3_step(stmt);
    assert(rc == SQLITE_ROW);

    count = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt); 
    free(query);

    return count;
}

wpd_error_t
get_wallpapers(const struct wpd_db_t  *db,
       struct wpd_wallpaper_result_set_t **result_set)
{
    const char                    *query;
    sqlite3_stmt                  *stmt;
    struct wpd_wallpaper_entity_t *wallpapers;
    int                            rc,
                                   wallpaper_count,
                                   i;

    wallpaper_count = get_wallpaper_count(db);
    assert(wallpaper_count > 0);

    wallpapers = malloc(wallpaper_count * sizeof(struct wpd_wallpaper_entity_t));

    query = "select path, width, height from wallpaper;";

    sqlite3_prepare_v2(db->handle, query, strlen(query), &stmt, NULL);  

    i = 0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        wallpapers[i].path = strdup((const char *)sqlite3_column_text(stmt, 0));
        wallpapers[i].width = sqlite3_column_int(stmt, 1);
        wallpapers[i].height = sqlite3_column_int(stmt, 2);

        i++;
    }

    sqlite3_finalize(stmt); 

    (*result_set) = malloc(sizeof(struct wpd_wallpaper_result_set_t));
    (*result_set)->results = wallpapers;
    (*result_set)->count = wallpaper_count;

    return WPD_ERROR_SUCCESS;
}

wpd_error_t
get_wallpapers_by_dimensions(const struct wpd_db_t *db, int width, int height,
       struct wpd_wallpaper_result_set_t **result_set)
{
    char                          *query;
    const char                    *query_template;
    sqlite3_stmt                  *stmt;
    struct wpd_wallpaper_entity_t *wallpapers;
    int                            rc,
                                   wallpaper_count,
                                   i;

    wallpaper_count = get_wallpaper_by_dimensions_count(db, width, height);
    assert(wallpaper_count > 0);

    wallpapers = malloc(wallpaper_count * sizeof(struct wpd_wallpaper_entity_t));

    query_template =
        "select "
        "    path, width, height "
        "from "
        "    wallpaper "
        "where "
        "    width = %u and height = %u;";

    asprintf(&query, query_template, width, height);

    sqlite3_prepare_v2(db->handle, query, strlen(query), &stmt, NULL);  

    for (i = 0; (rc = sqlite3_step(stmt)) == SQLITE_ROW; i++)
    {
        wallpapers[i].path = strdup((const char *)sqlite3_column_text(stmt, 0));
        wallpapers[i].width = sqlite3_column_int(stmt, 1);
        wallpapers[i].height = sqlite3_column_int(stmt, 2);
    }

    sqlite3_finalize(stmt); 
    free(query);

    (*result_set) = malloc(sizeof(struct wpd_wallpaper_result_set_t));
    (*result_set)->results = wallpapers;
    (*result_set)->count = wallpaper_count;

    return WPD_ERROR_SUCCESS;
}

wpd_error_t
cleanup_database(struct wpd_db_t** db)
{
    assert(db);
    assert(*db);

    sqlite3_close((*db)->handle);
    free(*db);

    return WPD_ERROR_SUCCESS;
}

//
// Private Helper Functions
//

wpd_error_t
create_tables(sqlite3 *db)
{
    int rc;

    rc = create_wallpaper_table(db);
    if (rc)
    {
        sqlite3_close(db);
        return WPD_ERROR_DATABASE_TABLE_CREATION_FAILURE;
    }

    return WPD_ERROR_SUCCESS;
}

int
create_wallpaper_table(sqlite3 *db)
{
    const char   *query;
    sqlite3_stmt *stmt;
    int           rc;

    query = 
        "create table wallpaper ("
            "wallpaper_id integer primary key,"
            "width integer not null,"
            "height integer not null,"
            "path text not null unique"
        ");";

    sqlite3_prepare_v2(db, query, -1, &stmt, NULL); 

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        printf("Failed to create wallpaper database table: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    sqlite3_finalize(stmt); 

    return 0;
}

