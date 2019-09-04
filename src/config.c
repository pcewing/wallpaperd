#include "config.h"
#include "log.h"
#include "parse.h"

#include <yaml.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static const struct wpd_config_t default_config = {
    .search_path_count = 0,
    .search_paths = NULL,
    .rotation = {
        .enabled = false,
        .frequency = 0
    }
};

static inline const char*
scalar(const yaml_node_t* node)
{
    return (const char*) node->data.scalar.value;
}

typedef wpd_error_t (*sequence_handler) (yaml_document_t* doc, yaml_node_t* node, struct wpd_config_t* config);
typedef wpd_error_t (*scalar_handler) (yaml_node_t* node, struct wpd_config_t* config);

struct map_item_handler {
    // The name of the key node
    const char *key;  

    // The expected type for the value node
    yaml_node_type_t type;

    // The handler used to process the map item, chosen based on type
    union {
        sequence_handler sequence;
        scalar_handler scalar;
        const struct map_item_handler* mapping;
    } data;
};

static const struct map_item_handler*
get_handler(const struct map_item_handler* handlers, const char* key)
{
    for (unsigned i = 0; handlers[i].key != NULL; ++i) {
        if (strcmp(handlers[i].key, key) == 0)
            return &handlers[i];
    }
    return NULL;
}

#define MAP_FOR_EACH(node, itr) \
    for (yaml_node_pair_t* itr = node->data.mapping.pairs.start; \
         itr < node->data.mapping.pairs.top; \
         itr++)

#define SEQUENCE_FOR_EACH(node, itr) \
    for (yaml_node_item_t *itr = node->data.sequence.items.start; \
         itr < node->data.sequence.items.top; \
         itr++)

#define SCALAR_HANDLER(property_name, property_handler) \
    {property_name, YAML_SCALAR_NODE, .data.scalar = property_handler}

#define SEQUENCE_HANDLER(property_name, property_handler) \
    {property_name, YAML_SEQUENCE_NODE, .data.sequence = property_handler}

#define MAPPING_HANDLER(property_name, property_handler) \
    {property_name, YAML_MAPPING_NODE, .data.mapping = property_handler}

/*
 * Rotation Config Handlers
 */

static wpd_error_t
handle_rotation_enabled(yaml_node_t* node, struct wpd_config_t* config)
{
    bool rotation_enabled;
    wpd_error_t error = parse_bool((char *)node->data.scalar.value,
            &rotation_enabled);
    if (error == WPD_ERROR_GLOBAL_SUCCESS)
        config->rotation.enabled = rotation_enabled;
    return error;
}

static wpd_error_t
handle_rotation_frequency(yaml_node_t* node, struct wpd_config_t* config)
{
    uint32_t rotation_frequency;
    wpd_error_t error = parse_uint32((char *)node->data.scalar.value,
            &rotation_frequency);
    if (error == WPD_ERROR_GLOBAL_SUCCESS)
        config->rotation.frequency = rotation_frequency;
    return error;
}

const struct map_item_handler rotation_handlers[] = {
    SCALAR_HANDLER("enabled", handle_rotation_enabled),
    SCALAR_HANDLER("frequency", handle_rotation_frequency),
    {NULL}
};

/*
 * Root Config Handlers
 */

static wpd_error_t
handle_search_paths(yaml_document_t* doc, yaml_node_t* node, struct wpd_config_t* config)
{
    if (config->search_paths != NULL)
    {
        LOGERROR("Duplicate definition of search_paths in config");
        return WPD_ERROR_CONFIG_DUPLICATE_FIELD_DEFINITION;
    }

    config->search_path_count = 0;
    SEQUENCE_FOR_EACH(node, itr) { ++config->search_path_count; }

    if (config->search_path_count == 0)
        return WPD_ERROR_GLOBAL_SUCCESS;

    config->search_paths = malloc(config->search_path_count * sizeof(char *));

    int i = 0;
    SEQUENCE_FOR_EACH(node, itr)
    {
        yaml_node_t *item = yaml_document_get_node(doc, *itr);
        assert(item->type == YAML_SCALAR_NODE);

        char *path = strdup((char *)item->data.scalar.value);
        assert(path);

        config->search_paths[i++] = path;
    }

    return WPD_ERROR_GLOBAL_SUCCESS;
}

const struct map_item_handler root_handlers[] = {
    SEQUENCE_HANDLER("search_paths", handle_search_paths),
    MAPPING_HANDLER("rotation", rotation_handlers),
    {NULL}
};

static wpd_error_t
handle_mapping(yaml_document_t* doc, yaml_node_t* node, const struct map_item_handler* handlers, struct wpd_config_t *config)
{
    assert(node->type == YAML_MAPPING_NODE);

    MAP_FOR_EACH(node, itr)
    {
        yaml_node_t *key = yaml_document_get_node(doc, itr->key);
        yaml_node_t *value = yaml_document_get_node(doc, itr->value);

        // All keys should be scalars
        assert(key->type == YAML_SCALAR_NODE);
        const struct map_item_handler *handler = get_handler(handlers, scalar(key));
        if (!handler)
            return WPD_ERROR_CONFIG_MISSING_YAML_MAP_ITEM_HANDLER;
        assert(value->type == handler->type);

        wpd_error_t error;
        switch (handler->type)
        {
            case YAML_MAPPING_NODE:
                assert(handler->data.mapping);
                error = handle_mapping(doc, value, handler->data.mapping, config);
                if (error != WPD_ERROR_GLOBAL_SUCCESS)
                    return error;
                break;

            case YAML_SEQUENCE_NODE:
                assert(handler->data.sequence);
                error = handler->data.sequence(doc, value, config);
                if (error != WPD_ERROR_GLOBAL_SUCCESS)
                    return error;
                break;

            case YAML_SCALAR_NODE:
                assert(handler->data.scalar);
                error = handler->data.scalar(value, config);
                if (error != WPD_ERROR_GLOBAL_SUCCESS)
                    return error;
                break;

            default:
                LOGWARN("Unknown handler type encountered in handle_mapping");
                break;
        };
    }

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t
load_yaml(const char *path, yaml_document_t* doc)
{
    FILE *input = fopen(path, "rb");
    if (!input)
        return WPD_ERROR_CONFIG_FILE_OPEN_FAILURE;

    yaml_parser_t parser;
    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, input);

    wpd_error_t error = WPD_ERROR_GLOBAL_SUCCESS;
    if (!yaml_parser_load(&parser, doc)) {
        error = WPD_ERROR_CONFIG_YAML_PARSER_LOAD_FAILURE;
    }

    yaml_parser_delete(&parser);

    fclose(input);
    return error;
}

wpd_error_t
parse_config(const char * path, struct wpd_config_t **config)
{
    wpd_error_t error = WPD_ERROR_GLOBAL_SUCCESS;
    yaml_document_t doc;

    error = load_yaml(path, &doc);
    if (error != WPD_ERROR_GLOBAL_SUCCESS)
    {
        yaml_document_delete(&doc);
        return error;
    }

    (*config) = memcpy(malloc(sizeof(struct wpd_config_t)), &default_config,
            sizeof(struct wpd_config_t));

    handle_mapping(&doc, yaml_document_get_root_node(&doc), root_handlers,
            *config);

    yaml_document_delete(&doc);
    return error;
}

wpd_error_t
locate_config(char **config_path)
{
    // First, look in the XDG config home if it's defined
    char *config_dir = NULL;
    char *config_home = getenv("XDG_CONFIG_HOME");
    if (config_home)
    {
        config_dir = wpd_path_join(config_home, "wallpaperd");
    }
    else
    {
        // Fall back to $HOME/.config
        char *home = getenv("HOME");
        if (!home)
            return WPD_ERROR_CONFIG_HOME_PATH_UNDEFINED;

        config_dir = wpd_path_join(home, ".config/wallpaperd");
    }

    (*config_path) = wpd_path_join(config_dir, "config.yaml");

    free(config_dir);
    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t
load_config(struct wpd_config_t** config)
{
    char *config_path = NULL;
    wpd_error_t error = locate_config(&config_path);
    if (error != WPD_ERROR_GLOBAL_SUCCESS)
        return error;

    error = parse_config(config_path, config);
    if (error != WPD_ERROR_GLOBAL_SUCCESS)
        return error;

    free(config_path);
    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t
destroy_config(struct wpd_config_t** config)
{
    assert(config);
    assert(*config);

    if ((*config)->search_path_count > 0)
    {
        for (uint32_t i = 0; i < (*config)->search_path_count; ++i)
        {
            free((*config)->search_paths[i]);
            (*config)->search_paths[i] = NULL;
        }

        free((*config)->search_paths);
        (*config)->search_paths = NULL;
    }

    free(*config);
    (*config) = NULL;

    return WPD_ERROR_GLOBAL_SUCCESS;
}
