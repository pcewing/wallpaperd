//
// !!! WARNING !!!
// 
// This file was auto-generated using the tool/codegen.py script. To make
// modifications, edit tool/error.xml and re-run that script.
//

#include <stdint.h>

typedef uint32_t wpd_error_t;

static const uint32_t WPD_ERROR_SUCCESS = 0;
static const uint32_t WPD_ERROR_FAILURE = 1;
static const uint32_t WPD_ERROR_UNKNOWN_FILENAME = 2;
static const uint32_t WPD_ERROR_UNKNOWN_EXTENSION = 3;
static const uint32_t WPD_ERROR_NULL_PARAM = 4;
static const uint32_t WPD_ERROR_TODO = 5;
static const uint32_t WPD_ERROR_NO_COMPATIBLE_IMAGE = 6;
static const uint32_t WPD_ERROR_DATABASE_TABLE_CREATION_FAILURE = 7;

const char *
wpd_error_str(wpd_error_t error);
