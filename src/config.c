#include "config.h"
#include "log.h"

#include <yaml.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static const struct wpd_config_t default_config = {
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

typedef wpd_error_t (*node_handler) (yaml_document_t* doc, yaml_node_t* node, struct wpd_config_t* config);

struct map_item_handler {
    // The name of the key node
    const char *key;  

    // The expected type for the value node
    yaml_node_type_t type;

    // TODO: Can we union the two fields below?

    // The handler function for non-mappingnodes
    node_handler handler;

    // The handler array for mapping nodes
    const struct map_item_handler* map_handlers;
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

bool
match(const char **haystack, const char *needle)
{
    for (int i = 0; haystack[i]; i++)
    {
        if (strcmp(needle, haystack[i]) == 0)
        {
            return true;
        }
    }
    return false;
}

wpd_error_t
parse_bool(const char *data, bool *result)
{
    static const char *t[] = { "true", "on", "yes", "y", NULL };
    static const char *f[] = { "false", "off", "no", "n", NULL };

    char *lower = wpd_strdup_lower(data);

    wpd_error_t error = WPD_ERROR_SUCCESS;

    if (match(t, lower))
        *result = true;
    else if (match(f, lower))
        *result = false;
    else
        error = WPD_ERROR_TODO;

    free(lower);
    return error;
}

/*
 * Rotation Config Handlers
 */

static wpd_error_t
handle_rotation_enabled(yaml_document_t* doc, yaml_node_t* node, struct wpd_config_t* config)
{
    UNUSED(doc);

    bool rotation_enabled;
    wpd_error_t error = parse_bool((char *)node->data.scalar.value, &rotation_enabled);
    if (error == WPD_ERROR_SUCCESS)
        config->rotation.enabled = rotation_enabled;
    return error;
}

static wpd_error_t
handle_rotation_frequency(yaml_document_t* doc, yaml_node_t* node, struct wpd_config_t* config)
{
    UNUSED(doc);

    config->rotation.frequency = atoi((char *)node->data.scalar.value);
    return WPD_ERROR_SUCCESS;
}

const struct map_item_handler rotation_handlers[] = {
    {"enabled", YAML_SCALAR_NODE, handle_rotation_enabled, NULL},
    {"frequency", YAML_SCALAR_NODE, handle_rotation_frequency, NULL},
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
        return WPD_ERROR_TODO;
    }

    int search_path_count = 0;
    SEQUENCE_FOR_EACH(node, itr) { ++search_path_count; }

    config->search_paths = malloc(search_path_count * sizeof(char *));

    int i = 0;
    SEQUENCE_FOR_EACH(node, itr)
    {
        yaml_node_t *item = yaml_document_get_node(doc, *itr);
        assert(item->type == YAML_SCALAR_NODE);

        char *path = strdup((char *)item->data.scalar.value);
        assert(path);

        config->search_paths[i++] = path;
    }

    return WPD_ERROR_SUCCESS;
}

const struct map_item_handler root_handlers[] = {
    {"search_paths", YAML_SEQUENCE_NODE, handle_search_paths, NULL,},
    {"rotation",     YAML_MAPPING_NODE,  NULL, rotation_handlers,},
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
        const struct map_item_handler *h = get_handler(handlers, scalar(key));
        if (!h)
            return WPD_ERROR_TODO;
        assert(value->type == h->type);
        if (h->map_handlers) {
            assert(h->handler == NULL);
            assert(h->type == YAML_MAPPING_NODE);
            if (handle_mapping(doc, value, h->map_handlers, config) != WPD_ERROR_SUCCESS)
                return WPD_ERROR_TODO;
        } else {
            if (h->handler(doc, value, config) != WPD_ERROR_SUCCESS)
                return WPD_ERROR_TODO;
        }
    }

    return WPD_ERROR_SUCCESS;
}

wpd_error_t
load_yaml(const char *path, yaml_document_t* doc)
{
    yaml_parser_t parser;
    wpd_error_t error = WPD_ERROR_SUCCESS;

    // TODO: Maybe open the file before this function is even called?
    FILE *input = fopen(path, "rb");
    if (!input)
        return WPD_ERROR_TODO;

    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, input);
    if (!yaml_parser_load(&parser, doc)) {
        error = WPD_ERROR_TODO;
        goto failure;
    }

failure:
    fclose(input);
    return error;
}

wpd_error_t
parse_config(const char * path, struct wpd_config_t **config)
{
    wpd_error_t error = WPD_ERROR_SUCCESS;
    yaml_document_t doc;

    error = load_yaml(path, &doc);
    if (error != WPD_ERROR_SUCCESS)
    {
        goto done;
    }

    (*config) = memcpy(malloc(sizeof(struct wpd_config_t)), &default_config,
            sizeof(struct wpd_config_t));

    handle_mapping(&doc, yaml_document_get_root_node(&doc), root_handlers, *config);

done:
    // TODO: Destroy the document?
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
            return WPD_ERROR_TODO;

        config_dir = wpd_path_join(home, ".config/wallpaperd");
    }

    (*config_path) = wpd_path_join(config_dir, "config.yaml");

    free(config_dir);
    return WPD_ERROR_SUCCESS;
}

wpd_error_t
load_config(struct wpd_config_t** config)
{
    char *config_path = NULL;
    wpd_error_t error = locate_config(&config_path);
    if (error != WPD_ERROR_SUCCESS)
        return error;

    error = parse_config(config_path, config);
    if (error != WPD_ERROR_SUCCESS)
        return error;

    free(config_path);
    // TODO: Switch this back
    return WPD_ERROR_TODO;
    //return WPD_ERROR_SUCCESS;
}

wpd_error_t
destroy_config(struct wpd_config_t** config)
{
    UNUSED(config);
    // TODO

    return WPD_ERROR_SUCCESS;
}
