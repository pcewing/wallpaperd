//
// !!! WARNING !!!
//
// This file was auto-generated using the tool/codegen.py script. To make
// modifications, edit tool/error.xml and re-run that script.
//

// clang-format off

#include "error.g.h"
#include <assert.h>

struct wpd_error_category_definition_t
{
    const char *str;
};

static struct wpd_error_category_definition_t error_category_definitions[] = {
    { "WPD_ERROR_CATEGORY_GLOBAL" },
    { "WPD_ERROR_CATEGORY_XCB" },
    { "WPD_ERROR_CATEGORY_FTW" },
    { "WPD_ERROR_CATEGORY_IMAGE" },
    { "WPD_ERROR_CATEGORY_DATA" },
    { "WPD_ERROR_CATEGORY_PARSE" },
    { "WPD_ERROR_CATEGORY_CONFIG" },
};

const char *
wpd_error_category_str(wpd_error_category_t error_category)
{
    return error_category_definitions[error_category].str;
}

struct wpd_error_definition_t
{
    wpd_error_category_t category;
    const char *str;
};

static struct wpd_error_definition_t error_definitions[] = {
    { WPD_ERROR_CATEGORY_GLOBAL, "WPD_ERROR_GLOBAL_SUCCESS" },
    { WPD_ERROR_CATEGORY_GLOBAL, "WPD_ERROR_GLOBAL_FAILURE" },
    { WPD_ERROR_CATEGORY_GLOBAL, "WPD_ERROR_GLOBAL_NULL_PARAM" },
    { WPD_ERROR_CATEGORY_XCB, "WPD_ERROR_XCB_GET_GEOMETRY_FAILED" },
    { WPD_ERROR_CATEGORY_FTW, "WPD_ERROR_FTW_UNKNOWN_EXTENSION" },
    { WPD_ERROR_CATEGORY_IMAGE, "WPD_ERROR_IMAGE_STBI_LOAD_FAILURE" },
    { WPD_ERROR_CATEGORY_DATA, "WPD_ERROR_DATA_SQLITE_OPEN_FAILURE" },
    { WPD_ERROR_CATEGORY_DATA, "WPD_ERROR_DATA_TABLE_CREATION_FAILURE" },
    { WPD_ERROR_CATEGORY_PARSE, "WPD_ERROR_PARSE_BOOL_INVALID" },
    { WPD_ERROR_CATEGORY_PARSE, "WPD_ERROR_PARSE_U32_NOT_A_DECIMAL" },
    { WPD_ERROR_CATEGORY_PARSE, "WPD_ERROR_PARSE_U32_TRAILING_CHARS" },
    { WPD_ERROR_CATEGORY_PARSE, "WPD_ERROR_PARSE_U32_STRTOULL_OUT_OF_RANGE" },
    { WPD_ERROR_CATEGORY_PARSE, "WPD_ERROR_PARSE_U32_OVERFLOW" },
    { WPD_ERROR_CATEGORY_CONFIG, "WPD_ERROR_CONFIG_DUPLICATE_FIELD_DEFINITION" },
    { WPD_ERROR_CATEGORY_CONFIG, "WPD_ERROR_CONFIG_MISSING_YAML_MAP_ITEM_HANDLER" },
    { WPD_ERROR_CATEGORY_CONFIG, "WPD_ERROR_CONFIG_FILE_OPEN_FAILURE" },
    { WPD_ERROR_CATEGORY_CONFIG, "WPD_ERROR_CONFIG_YAML_PARSER_LOAD_FAILURE" },
    { WPD_ERROR_CATEGORY_CONFIG, "WPD_ERROR_CONFIG_HOME_PATH_UNDEFINED" },
};

const char *
wpd_error_str(wpd_error_t error)
{
    return error_definitions[error].str;
}
