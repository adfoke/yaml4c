#include "yaml4c.h"
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <yaml_file>\n", argv[0]);
        return 1;
    }

    y4c_error_t err;
    y4c_node_t *root = y4c_load_file(argv[1], &err);
    if (!root) {
        fprintf(stderr, "Failed to parse YAML file: %s (Line: %zu, Col: %zu)\n", 
                err.message, err.line + 1, err.column + 1);
        return 1;
    }

    // Example usage assuming a structure like:
    // app:
    //   name: "MyApp"
    //   version: 1.0
    //   enabled: true
    //   ports:
    //     - 8080
    //     - 9090

    y4c_node_t *app = y4c_get(root, "app");
    if (app) {
        const char *name = y4c_get_str(app, "name", "Unknown");
        double version = y4c_get_double(app, "version", 0.0);
        bool enabled = y4c_get_bool(app, "enabled", false);
        
        printf("App Name: %s\n", name);
        printf("Version: %.1f\n", version);
        printf("Enabled: %s\n", enabled ? "true" : "false");
        
        y4c_node_t *ports = y4c_get(app, "ports");
        if (ports && ports->type == Y4C_SEQUENCE) {
            printf("Ports:\n");
            int i = 0;
            y4c_node_t *port_node;
            while ((port_node = y4c_at(ports, i++))) {
                if (port_node->type == Y4C_SCALAR) {
                    printf("  - %s\n", port_node->value);
                }
            }
        }
    } else {
        printf("Root 'app' node not found. Dumping top-level keys:\n");
        if (root->type == Y4C_MAPPING) {
            y4c_node_t *curr = root->head;
            while (curr) {
                printf("Key: %s, Value: %s\n", curr->key, curr->value ? curr->value : "(complex)");
                curr = curr->next;
            }
        }
    }

    y4c_free(root);
    return 0;
}
