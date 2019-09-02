//
// !!! WARNING !!!
// 
// This file was auto-generated using the tool/codegen.py script. To make
// modifications, edit tool/error.xml and re-run that script.
//

#ifndef ERROR_G_H
#define ERROR_G_H

#include <stdint.h>

typedef uint32_t wpd_error_category_t;

static const wpd_error_category_t WPD_ERROR_CATEGORY_GLOBAL = 0;
static const wpd_error_category_t WPD_ERROR_CATEGORY_XCB = 1;
static const wpd_error_category_t WPD_ERROR_CATEGORY_FTW = 2;
static const wpd_error_category_t WPD_ERROR_CATEGORY_IMAGE = 3;
static const wpd_error_category_t WPD_ERROR_CATEGORY_DATA = 4;
static const wpd_error_category_t WPD_ERROR_CATEGORY_PARSE = 5;
static const wpd_error_category_t WPD_ERROR_CATEGORY_CONFIG = 6;

const char *
wpd_error_category_str(wpd_error_category_t error_category);

typedef uint32_t wpd_error_t;

static const wpd_error_t WPD_ERROR_GLOBAL_SUCCESS = 0;
static const wpd_error_t WPD_ERROR_GLOBAL_FAILURE = 1;
static const wpd_error_t WPD_ERROR_GLOBAL_NULL_PARAM = 2;
static const wpd_error_t WPD_ERROR_XCB_GET_GEOMETRY_FAILED = 3;
static const wpd_error_t WPD_ERROR_FTW_UNKNOWN_EXTENSION = 4;
static const wpd_error_t WPD_ERROR_IMAGE_STBI_LOAD_FAILURE = 5;
static const wpd_error_t WPD_ERROR_DATA_SQLITE_OPEN_FAILURE = 6;
static const wpd_error_t WPD_ERROR_DATA_TABLE_CREATION_FAILURE = 7;
static const wpd_error_t WPD_ERROR_PARSE_BOOL_INVALID = 8;
static const wpd_error_t WPD_ERROR_PARSE_U32_NOT_A_DECIMAL = 9;
static const wpd_error_t WPD_ERROR_PARSE_U32_TRAILING_CHARS = 10;
static const wpd_error_t WPD_ERROR_PARSE_U32_STRTOULL_OUT_OF_RANGE = 11;
static const wpd_error_t WPD_ERROR_PARSE_U32_OVERFLOW = 12;
static const wpd_error_t WPD_ERROR_CONFIG_DUPLICATE_FIELD_DEFINITION = 13;
static const wpd_error_t WPD_ERROR_CONFIG_MISSING_YAML_MAP_ITEM_HANDLER = 14;
static const wpd_error_t WPD_ERROR_CONFIG_FILE_OPEN_FAILURE = 15;
static const wpd_error_t WPD_ERROR_CONFIG_YAML_PARSER_LOAD_FAILURE = 16;
static const wpd_error_t WPD_ERROR_CONFIG_HOME_PATH_UNDEFINED = 17;

const char *
wpd_error_str(wpd_error_t error);

#endif // ERROR_G_H
