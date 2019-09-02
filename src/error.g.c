//
// !!! WARNING !!!
// 
// This file was auto-generated using the tool/codegen.py script. To make
// modifications, edit tool/error.xml and re-run that script.
//

#include "error.g.h"
#include <assert.h>

const char *
wpd_error_str(wpd_error_t error)
{
    switch (error)
    {
    case 0: // WPD_ERROR_SUCCESS
        return "WPD_ERROR_SUCCESS";
    case 1: // WPD_ERROR_FAILURE
        return "WPD_ERROR_FAILURE";
    case 2: // WPD_ERROR_UNKNOWN_FILENAME
        return "WPD_ERROR_UNKNOWN_FILENAME";
    case 3: // WPD_ERROR_UNKNOWN_EXTENSION
        return "WPD_ERROR_UNKNOWN_EXTENSION";
    case 4: // WPD_ERROR_NULL_PARAM
        return "WPD_ERROR_NULL_PARAM";
    case 5: // WPD_ERROR_TODO
        return "WPD_ERROR_TODO";
    case 6: // WPD_ERROR_NO_COMPATIBLE_IMAGE
        return "WPD_ERROR_NO_COMPATIBLE_IMAGE";
    case 7: // WPD_ERROR_DATABASE_TABLE_CREATION_FAILURE
        return "WPD_ERROR_DATABASE_TABLE_CREATION_FAILURE";
    default:
        assert(0);
    }
}