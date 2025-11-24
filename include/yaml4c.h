#ifndef YAML4C_H
#define YAML4C_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 3. Data Structures */

typedef enum {
    Y4C_NULL = 0,
    Y4C_SCALAR,    // Scalar: string, number, boolean
    Y4C_SEQUENCE,  // Sequence: list/array ([...])
    Y4C_MAPPING    // Mapping: key-value pairs ({...})
} y4c_type_t;

typedef struct y4c_node {
    y4c_type_t type;
    
    char *key;             // Node name (exists only if this node is a child of MAPPING)
    char *value;           // Node value (exists only if type == Y4C_SCALAR)
    
    struct y4c_node *head; // Child node list head (for SEQUENCE or MAPPING)
    struct y4c_node *tail; // Child node list tail (for O(1) insertion)
    
    struct y4c_node *next; // Next sibling node
    struct y4c_node *prev; // Previous sibling node
} y4c_node_t;

/* 4. Core API Contract */

/* 4.1 Basic Operations */
/**
 * Read file and parse into a tree.
 * @param path File path
 * @return Root node of the parsed tree, or NULL on failure
 */
y4c_node_t *y4c_load_file(const char *path);

/**
 * Parse from string.
 * @param str YAML string
 * @param len Length of the string
 * @return Root node of the parsed tree, or NULL on failure
 */
y4c_node_t *y4c_load_str(const char *str, size_t len);

/**
 * Recursively free the node and all its children.
 * @param node Node to free
 */
void y4c_free(y4c_node_t *node);

/* 4.2 Node Access */

/**
 * Get child node by key (for Mapping).
 * @param node Parent node (must be MAPPING)
 * @param key Key string
 * @return Child node if found, otherwise NULL
 */
y4c_node_t *y4c_get(y4c_node_t *node, const char *key);

/**
 * Get child node by index (for Sequence).
 * @param node Parent node (must be SEQUENCE)
 * @param index Index (0-based)
 * @return Child node if found, otherwise NULL
 */
y4c_node_t *y4c_at(y4c_node_t *node, int index);

/* 4.3 Type Safe Helpers */

/**
 * Get string value helper.
 * @param node Parent node
 * @param key Key to look up (if NULL, checks the node itself)
 * @param def Default value if not found or type mismatch
 * @return String value
 */
const char *y4c_get_str(y4c_node_t *node, const char *key, const char *def);

/**
 * Get integer value helper.
 * @param node Parent node
 * @param key Key to look up
 * @param def Default value
 * @return Integer value
 */
int y4c_get_int(y4c_node_t *node, const char *key, int def);

/**
 * Get boolean value helper.
 * @param node Parent node
 * @param key Key to look up
 * @param def Default value
 * @return Boolean value
 */
bool y4c_get_bool(y4c_node_t *node, const char *key, bool def);

/**
 * Get double value helper.
 * @param node Parent node
 * @param key Key to look up
 * @param def Default value
 * @return Double value
 */
double y4c_get_double(y4c_node_t *node, const char *key, double def);

#ifdef __cplusplus
}
#endif

#endif // YAML4C_H
