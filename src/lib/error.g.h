//
// !!! WARNING !!!
//
// This file was auto-generated using the tool/codegen.py script. To make
// modifications, edit tool/error.xml and re-run that script.
//

// clang-format off

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
static const wpd_error_category_t WPD_ERROR_CATEGORY_IPC = 7;
static const wpd_error_category_t WPD_ERROR_CATEGORY_RPC = 8;

const char *
wpd_error_category_str(wpd_error_category_t error_category);

typedef uint32_t wpd_error_t;

static const wpd_error_t WPD_ERROR_GLOBAL_SUCCESS = 0;
static const wpd_error_t WPD_ERROR_GLOBAL_FAILURE = 1;
static const wpd_error_t WPD_ERROR_GLOBAL_NULL_PARAM = 2;
static const wpd_error_t WPD_ERROR_GLOBAL_ALREADY_RUNNING = 3;
static const wpd_error_t WPD_ERROR_XCB_GET_GEOMETRY_FAILED = 4;
static const wpd_error_t WPD_ERROR_FTW_UNKNOWN_EXTENSION = 5;
static const wpd_error_t WPD_ERROR_IMAGE_STBI_LOAD_FAILURE = 6;
static const wpd_error_t WPD_ERROR_DATA_SQLITE_OPEN_FAILURE = 7;
static const wpd_error_t WPD_ERROR_DATA_TABLE_CREATION_FAILURE = 8;
static const wpd_error_t WPD_ERROR_PARSE_BOOL_INVALID = 9;
static const wpd_error_t WPD_ERROR_PARSE_U32_NOT_A_DECIMAL = 10;
static const wpd_error_t WPD_ERROR_PARSE_U32_TRAILING_CHARS = 11;
static const wpd_error_t WPD_ERROR_PARSE_U32_STRTOULL_OUT_OF_RANGE = 12;
static const wpd_error_t WPD_ERROR_PARSE_U32_OVERFLOW = 13;
static const wpd_error_t WPD_ERROR_CONFIG_DUPLICATE_FIELD_DEFINITION = 14;
static const wpd_error_t WPD_ERROR_CONFIG_MISSING_YAML_MAP_ITEM_HANDLER = 15;
static const wpd_error_t WPD_ERROR_CONFIG_FILE_OPEN_FAILURE = 16;
static const wpd_error_t WPD_ERROR_CONFIG_FILE_DOES_NOT_EXIST = 17;
static const wpd_error_t WPD_ERROR_CONFIG_YAML_PARSER_LOAD_FAILURE = 18;
static const wpd_error_t WPD_ERROR_CONFIG_HOME_PATH_UNDEFINED = 19;
static const wpd_error_t WPD_ERROR_IPC_FAILED_TO_CREATE_SOCKET = 20;
static const wpd_error_t WPD_ERROR_IPC_FAILED_TO_GET_FLAGS = 21;
static const wpd_error_t WPD_ERROR_IPC_FAILED_TO_SET_FLAGS = 22;
static const wpd_error_t WPD_ERROR_IPC_FAILED_TO_CONNECT = 23;
static const wpd_error_t WPD_ERROR_IPC_INVALID_ADDRESS_PATH_LENGTH = 24;
static const wpd_error_t WPD_ERROR_IPC_FAILED_TO_REMOVE_SOCKET = 25;
static const wpd_error_t WPD_ERROR_IPC_FAILED_TO_BIND_SOCKET = 26;
static const wpd_error_t WPD_ERROR_IPC_FAILED_TO_LISTEN_ON_SOCKET = 27;
static const wpd_error_t WPD_ERROR_IPC_FAILED_TO_WRITE_TO_SOCKET = 28;
static const wpd_error_t WPD_ERROR_IPC_UNEXPECTED_WRITE_LENGTH = 29;
static const wpd_error_t WPD_ERROR_IPC_FAILED_TO_READ_FROM_SOCKET = 30;
static const wpd_error_t WPD_ERROR_IPC_UNEXPECTED_READ_LENGTH = 31;
static const wpd_error_t WPD_ERROR_IPC_FAILED_TO_ACCEPT_CONNECTION = 32;
static const wpd_error_t WPD_ERROR_IPC_FAILED_TO_CLOSE_SOCKET = 33;
static const wpd_error_t WPD_ERROR_RPC_UNEXPECTED_MESSAGE_TYPE = 34;
static const wpd_error_t WPD_ERROR_RPC_UNEXPECTED_RESPONSE_TYPE = 35;
static const wpd_error_t WPD_ERROR_RPC_REQUEST_TYPE_UNKNOWN = 36;
static const wpd_error_t WPD_ERROR_RPC_INVALID_MESSAGE = 37;
static const wpd_error_t WPD_ERROR_RPC_MISSING_REPONSE_ALLOCATOR = 38;

const char *
wpd_error_str(wpd_error_t error);

#endif // ERROR_G_H
