//
// !!! WARNING !!!
// 
// This file was auto-generated using the tool/codegen.py script. To make
// modifications, edit tool/error.xml and re-run that script.
//

#ifndef ERROR_G_H
#define ERROR_G_H

#include <stdint.h>

typedef uint32_t wpd_error_t;

static const wpd_error_t WPD_ERROR_SUCCESS = 0;
static const wpd_error_t WPD_ERROR_FAILURE = 1;
static const wpd_error_t WPD_ERROR_XCB_GET_GEOMETRY_FAILED = 2;
static const wpd_error_t WPD_ERROR_UNKNOWN_FILENAME = 3;
static const wpd_error_t WPD_ERROR_UNKNOWN_EXTENSION = 4;
static const wpd_error_t WPD_ERROR_NULL_PARAM = 5;
static const wpd_error_t WPD_ERROR_TODO = 6;
static const wpd_error_t WPD_ERROR_NO_COMPATIBLE_IMAGE = 7;
static const wpd_error_t WPD_ERROR_STBI_IMAGE_LOAD_FAILURE = 8;
static const wpd_error_t WPD_ERROR_DATABASE_OPEN_FAILURE = 9;
static const wpd_error_t WPD_ERROR_DATABASE_TABLE_CREATION_FAILURE = 10;
static const wpd_error_t WPD_ERROR_PARSE_FAILURE = 11;
static const wpd_error_t WPD_ERROR_DUPLICATE_CONFIG_DEFINITION = 12;
static const wpd_error_t WPD_ERROR_MISSING_YAML_MAP_ITEM_HANDLER = 13;
static const wpd_error_t WPD_ERROR_FILE_OPEN_FAILURE = 14;
static const wpd_error_t WPD_ERROR_YAML_PARSER_LOAD_FAILURE = 15;
static const wpd_error_t WPD_ERROR_HOME_PATH_UNDEFINED = 16;

const char *
wpd_error_str(wpd_error_t error);

#endif // ERROR_G_H
