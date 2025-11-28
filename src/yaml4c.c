#include "yaml4c.h"
#include <yaml.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // for strcasecmp

/* Helper: Set error details */
static void y4c_set_error(y4c_error_t *err, const char *msg, size_t line, size_t col) {
    if (err) {
        snprintf(err->message, sizeof(err->message), "%s", msg ? msg : "Unknown error");
        err->line = line;
        err->column = col;
    }
}

/* Helper: Create new node */
static y4c_node_t *y4c_new_node(y4c_type_t type) {
    y4c_node_t *node = (y4c_node_t *)calloc(1, sizeof(y4c_node_t));
    if (node) {
        node->type = type;
    }
    return node;
}

/* Helper: Append child to parent */
static void y4c_append_child(y4c_node_t *parent, y4c_node_t *child) {
    if (!parent || !child) return;
    
    if (parent->tail) {
        parent->tail->next = child;
        child->prev = parent->tail;
        parent->tail = child;
    } else {
        parent->head = child;
        parent->tail = child;
    }
}

/* Recursive Event Parser */
static y4c_node_t *y4c_parse_event(yaml_parser_t *parser, yaml_event_t *input_event) {
    y4c_node_t *node = NULL;
    
    switch (input_event->type) {
        case YAML_SCALAR_EVENT:
            node = y4c_new_node(Y4C_SCALAR);
            if (node) {
                node->value = strdup((char *)input_event->data.scalar.value);
            }
            break;
            
        case YAML_SEQUENCE_START_EVENT:
            node = y4c_new_node(Y4C_SEQUENCE);
            if (!node) break;
            
            while (1) {
                yaml_event_t event;
                if (!yaml_parser_parse(parser, &event)) break;
                
                if (event.type == YAML_SEQUENCE_END_EVENT) {
                    yaml_event_delete(&event);
                    break;
                }
                
                y4c_node_t *child = y4c_parse_event(parser, &event);
                yaml_event_delete(&event);
                
                if (child) {
                    y4c_append_child(node, child);
                }
            }
            break;
            
        case YAML_MAPPING_START_EVENT:
            node = y4c_new_node(Y4C_MAPPING);
            if (!node) break;
            
            while (1) {
                yaml_event_t key_event;
                if (!yaml_parser_parse(parser, &key_event)) break;
                
                if (key_event.type == YAML_MAPPING_END_EVENT) {
                    yaml_event_delete(&key_event);
                    break;
                }
                
                char *key = NULL;
                if (key_event.type == YAML_SCALAR_EVENT) {
                    key = strdup((char *)key_event.data.scalar.value);
                }
                yaml_event_delete(&key_event);
                
                yaml_event_t val_event;
                if (!yaml_parser_parse(parser, &val_event)) {
                    if (key) free(key);
                    break;
                }
                
                y4c_node_t *child = y4c_parse_event(parser, &val_event);
                yaml_event_delete(&val_event);
                
                if (child) {
                    if (key) child->key = key;
                    else if (key) free(key); // Should not happen if logic correct
                    y4c_append_child(node, child);
                } else {
                    if (key) free(key);
                }
            }
            break;
            
        default:
            break;
    }
    
    return node;
}

y4c_node_t *y4c_load_file(const char *path, y4c_error_t *err) {
    if (err) memset(err, 0, sizeof(y4c_error_t));

    FILE *fh = fopen(path, "r");
    if (!fh) {
        y4c_set_error(err, "Failed to open file", 0, 0);
        return NULL;
    }
    
    yaml_parser_t parser;
    if (!yaml_parser_initialize(&parser)) {
        fclose(fh);
        y4c_set_error(err, "Failed to initialize yaml parser", 0, 0);
        return NULL;
    }
    
    yaml_parser_set_input_file(&parser, fh);
    
    y4c_node_t *root = NULL;
    bool done = false;
    
    while (!done) {
        yaml_event_t event;
        if (!yaml_parser_parse(&parser, &event)) {
            y4c_set_error(err, parser.problem, parser.problem_mark.line, parser.problem_mark.column);
            if (root) {
                y4c_free(root);
                root = NULL;
            }
            break;
        }
        
        if (event.type == YAML_STREAM_END_EVENT) {
            done = true;
        } else if (event.type == YAML_DOCUMENT_START_EVENT) {
            yaml_event_t root_event;
            if (yaml_parser_parse(&parser, &root_event)) {
                root = y4c_parse_event(&parser, &root_event);
                yaml_event_delete(&root_event);
                done = true; // Only parse first document
            }
        }
        
        yaml_event_delete(&event);
    }
    
    yaml_parser_delete(&parser);
    fclose(fh);
    return root;
}

y4c_node_t *y4c_load_str(const char *str, size_t len, y4c_error_t *err) {
    if (err) memset(err, 0, sizeof(y4c_error_t));

    yaml_parser_t parser;
    if (!yaml_parser_initialize(&parser)) {
        y4c_set_error(err, "Failed to initialize yaml parser", 0, 0);
        return NULL;
    }
    
    yaml_parser_set_input_string(&parser, (const unsigned char *)str, len);
    
    y4c_node_t *root = NULL;
    bool done = false;
    
    while (!done) {
        yaml_event_t event;
        if (!yaml_parser_parse(&parser, &event)) {
            y4c_set_error(err, parser.problem, parser.problem_mark.line, parser.problem_mark.column);
            if (root) {
                y4c_free(root);
                root = NULL;
            }
            break;
        }
        
        if (event.type == YAML_STREAM_END_EVENT) {
            done = true;
        } else if (event.type == YAML_DOCUMENT_START_EVENT) {
            yaml_event_t root_event;
            if (yaml_parser_parse(&parser, &root_event)) {
                root = y4c_parse_event(&parser, &root_event);
                yaml_event_delete(&root_event);
                done = true;
            }
        }
        
        yaml_event_delete(&event);
    }
    
    yaml_parser_delete(&parser);
    return root;
}

void y4c_free(y4c_node_t *node) {
    if (!node) return;
    
    y4c_node_t *curr = node->head;
    while (curr) {
        y4c_node_t *next = curr->next;
        y4c_free(curr);
        curr = next;
    }
    
    if (node->key) free(node->key);
    if (node->value) free(node->value);
    free(node);
}

y4c_node_t *y4c_get(y4c_node_t *node, const char *key) {
    if (!node || node->type != Y4C_MAPPING || !key) return NULL;
    
    y4c_node_t *curr = node->head;
    while (curr) {
        if (curr->key && strcmp(curr->key, key) == 0) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

y4c_node_t *y4c_at(y4c_node_t *node, int index) {
    if (!node || node->type != Y4C_SEQUENCE || index < 0) return NULL;
    
    y4c_node_t *curr = node->head;
    int i = 0;
    while (curr) {
        if (i == index) return curr;
        curr = curr->next;
        i++;
    }
    return NULL;
}

const char *y4c_get_str(y4c_node_t *node, const char *key, const char *def) {
    y4c_node_t *target = node;
    if (key) {
        target = y4c_get(node, key);
    }
    
    if (target && target->type == Y4C_SCALAR && target->value) {
        return target->value;
    }
    return def;
}

int y4c_get_int(y4c_node_t *node, const char *key, int def) {
    const char *val = y4c_get_str(node, key, NULL);
    if (val) {
        return atoi(val);
    }
    return def;
}

bool y4c_get_bool(y4c_node_t *node, const char *key, bool def) {
    const char *val = y4c_get_str(node, key, NULL);
    if (val) {
        if (strcasecmp(val, "true") == 0 || 
            strcasecmp(val, "yes") == 0 || 
            strcasecmp(val, "on") == 0 || 
            strcmp(val, "1") == 0) {
            return true;
        }
        if (strcasecmp(val, "false") == 0 || 
            strcasecmp(val, "no") == 0 || 
            strcasecmp(val, "off") == 0 || 
            strcmp(val, "0") == 0) {
            return false;
        }
    }
    return def;
}

double y4c_get_double(y4c_node_t *node, const char *key, double def) {
    const char *val = y4c_get_str(node, key, NULL);
    if (val) {
        return atof(val);
    }
    return def;
}
