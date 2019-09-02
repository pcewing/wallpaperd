//
// !!! WARNING !!!
// 
// This file was auto-generated using the tool/codegen.py script. To make
// modifications, edit tool/error.xml and re-run that script.
//

#include "error.g.h"
#include <assert.h>

static const char * error_strings[] = {
    "WPD_ERROR_SUCCESS",
    "WPD_ERROR_FAILURE",
    "WPD_ERROR_XCB_GET_GEOMETRY_FAILED",
    "WPD_ERROR_UNKNOWN_FILENAME",
    "WPD_ERROR_UNKNOWN_EXTENSION",
    "WPD_ERROR_NULL_PARAM",
    "WPD_ERROR_TODO",
    "WPD_ERROR_NO_COMPATIBLE_IMAGE",
    "WPD_ERROR_STBI_IMAGE_LOAD_FAILURE",
    "WPD_ERROR_DATABASE_OPEN_FAILURE",
    "WPD_ERROR_DATABASE_TABLE_CREATION_FAILURE",
    "WPD_ERROR_PARSE_FAILURE",
    "WPD_ERROR_DUPLICATE_CONFIG_DEFINITION",
    "WPD_ERROR_MISSING_YAML_MAP_ITEM_HANDLER",
    "WPD_ERROR_FILE_OPEN_FAILURE",
    "WPD_ERROR_YAML_PARSER_LOAD_FAILURE",
    "WPD_ERROR_HOME_PATH_UNDEFINED",
};

const char *
wpd_error_str(wpd_error_t error)
{
    return error_strings[error];
}
