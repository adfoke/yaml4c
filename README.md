# yaml4c

A lightweight, DOM-like YAML parsing library wrapper for C, built on top of [libyaml](https://github.com/yaml/libyaml).

`yaml4c` abstracts away the complex event-based parsing of `libyaml`, providing a simple tree structure API similar to cJSON. It allows developers to easily load, navigate, and access data from YAML files without dealing with parser state machines.

## Features

- **Simple API**: DOM-style tree structure (Node-based) for easy navigation.
- **Defensive Design**: Null-safe operations and type-safe helpers with default values.
- **Unified Memory Management**: Automatic recursive memory freeing with a single call.
- **Lightweight**: Minimal overhead over `libyaml`.

## Dependencies

- **libyaml**: The underlying YAML parser.
- **CMake**: For building the project.

## Build Instructions

### 1. Install Dependencies

Ensure `libyaml` is installed on your system.

**macOS (Homebrew):**
```bash
brew install libyaml
```

**Ubuntu/Debian:**
```bash
sudo apt-get install libyaml-dev
```

### 2. Build with CMake

```bash
mkdir build
cd build
cmake ..
make
```

This will generate the `libyaml4c` shared library and the `demo` executable.

## Usage

### Loading YAML

You can load YAML from a file or a string.

```c
#include "yaml4c.h"

// Load from file
y4c_node_t *root = y4c_load_file("config.yaml");

// Or load from string
// y4c_node_t *root = y4c_load_str(yaml_string, len);

if (!root) {
    // Handle error
    fprintf(stderr, "Failed to load YAML\n");
}
```

### Accessing Data

Suppose you have a YAML file:

```yaml
app:
  name: "MyApp"
  version: 1.0
  enabled: true
  ports:
    - 8080
    - 9090
```

You can access it using helper functions that allow specifying default values (returned if the key is missing or the type doesn't match):

```c
// Get 'app' node (returns NULL if not found)
y4c_node_t *app = y4c_get(root, "app");

// Type-safe helpers with default values
const char *name = y4c_get_str(app, "name", "Unknown");
double version = y4c_get_double(app, "version", 0.0);
bool enabled = y4c_get_bool(app, "enabled", false);

printf("App: %s, Version: %.1f\n", name, version);

// Access sequences
y4c_node_t *ports = y4c_get(app, "ports");
if (ports) {
    // Use y4c_at to access by index
    y4c_node_t *p1 = y4c_at(ports, 0); // 8080
    y4c_node_t *p2 = y4c_at(ports, 1); // 9090
    
    if (p1 && p1->type == Y4C_SCALAR) {
        printf("Port 1: %s\n", p1->value);
    }
}
```

### Cleanup

Free the entire tree with one call:

```c
y4c_free(root);
```

## API Reference

### Data Structures

```c
typedef enum {
    Y4C_NULL,
    Y4C_SCALAR,    // String, number, boolean
    Y4C_SEQUENCE,  // List/Array ([...])
    Y4C_MAPPING    // Key-Value pairs ({...})
} y4c_type_t;

typedef struct y4c_node {
    y4c_type_t type;
    char *key;             // Node key (exists only if this node is a child of MAPPING)
    char *value;           // Node value (exists only if type == Y4C_SCALAR)
    struct y4c_node *head; // Child node list head
    struct y4c_node *next; // Next sibling node
    // ...
} y4c_node_t;
```

### Core Functions

- `y4c_node_t *y4c_load_file(const char *path)`: Read file and parse into a tree.
- `y4c_node_t *y4c_load_str(const char *str, size_t len)`: Parse from string.
- `void y4c_free(y4c_node_t *node)`: Recursively free the node and all its children.

### Navigation & Access

- `y4c_node_t *y4c_get(y4c_node_t *node, const char *key)`: Get child node by key (for Mapping).
- `y4c_node_t *y4c_at(y4c_node_t *node, int index)`: Get child node by index (for Sequence).

### Helper Functions

These functions provide a safe way to retrieve values, returning the provided `def` (default) value if the node doesn't exist, the key isn't found, or the value type is incompatible.

- `const char *y4c_get_str(y4c_node_t *node, const char *key, const char *def)`
- `int y4c_get_int(y4c_node_t *node, const char *key, int def)`
- `bool y4c_get_bool(y4c_node_t *node, const char *key, bool def)`
- `double y4c_get_double(y4c_node_t *node, const char *key, double def)`

## License

MIT License. See [LICENSE](LICENSE) for details.
