Project Context: yaml4c
1. 项目概述 (Project Overview)
yaml4c 是一个轻量级的 C 语言 YAML 解析库封装。

核心目标：屏蔽底层 libyaml 繁琐的基于事件（Event-based）的解析细节，向用户提供一个简单的、类似 DOM 的树形结构 API（类似于 cJSON 或 cYAML）。

底层依赖：libyaml (必须安装此动态库)。

开发语言：C (C99/C11 标准)。

构建系统：CMake。

2. 设计哲学 (Design Philosophy)
易用性优先 (Ease of Use)：用户不需要了解 YAML 解析器的状态机，只需操作节点树。

防御性编程 (Defensive Coding)：所有 API 必须检查空指针；内存分配失败需优雅返回；提供类型安全的获取函数。

统一内存管理 (Unified Memory)：

解析时：库负责申请内存（strdup 字符串）。

释放时：用户调用 y4c_free(root)，库负责递归释放整棵树。

3. 数据结构定义 (Data Structures)
核心节点结构体 y4c_node 必须遵循以下设计（链表式树结构）：

C

typedef enum {
    Y4C_NULL = 0,
    Y4C_SCALAR,    // 标量：字符串、数字、布尔
    Y4C_SEQUENCE,  // 序列：列表/数组 ([...])
    Y4C_MAPPING    // 映射：键值对 ({...})
} y4c_type_t;

typedef struct y4c_node {
    y4c_type_t type;
    
    char *key;             // 节点名称 (仅当该节点是 MAPPING 的子节点时存在)
    char *value;           // 节点值 (仅当 type == Y4C_SCALAR 时存在)
    
    struct y4c_node *head; // 子节点链表头 (用于 SEQUENCE 或 MAPPING)
    struct y4c_node *tail; // 子节点链表尾 (方便 O(1) 插入)
    
    struct y4c_node *next; // 同级下一个节点
    struct y4c_node *prev; // 同级上一个节点
} y4c_node_t;
4. 核心 API 规范 (API Contract)
4.1 基础操作
y4c_node_t *y4c_load_file(const char *path); - 读取文件并解析为树。

y4c_node_t *y4c_load_str(const char *str, size_t len); - 从字符串解析。

void y4c_free(y4c_node_t *node); - 递归释放节点及其所有子节点。

4.2 节点访问
y4c_node_t *y4c_get(y4c_node_t *node, const char *key); - 获取 Map 下的子节点。

y4c_node_t *y4c_at(y4c_node_t *node, int index); - 获取 Sequence 下的第 N 个节点。

4.3 类型安全获取 (Helper Functions)
所有 Helper 函数必须支持默认值 (def)，当节点不存在或类型不匹配时返回默认值。

const char *y4c_get_str(y4c_node_t *node, const char *key, const char *def);

int y4c_get_int(y4c_node_t *node, const char *key, int def);

bool y4c_get_bool(y4c_node_t *node, const char *key, bool def);

double y4c_get_double(y4c_node_t *node, const char *key, double def);

5. 实现细节指南 (Implementation Details)
5.1 解析逻辑 (Parsing Logic)
使用 libyaml 的 Parser API (Event-based)，不要使用 Document/Tree API。

维护一个递归函数 parse_recurse(yaml_parser_t *parser)。

状态机映射：

YAML_SCALAR_EVENT -> 创建 Y4C_SCALAR 节点。

YAML_SEQUENCE_START_EVENT -> 创建 Y4C_SEQUENCE 节点，循环递归直到 SEQUENCE_END。

YAML_MAPPING_START_EVENT -> 创建 Y4C_MAPPING 节点，循环递归（先取 Key Scalar，再递归取 Value）直到 MAPPING_END。

5.2 错误处理
如果 libyaml 解析出错，应打印 stderr 错误信息，并释放已分配的部分内存，返回 NULL。

必须处理 yaml_parser_initialize 失败的情况。

5.3 构建规范 (Build System)
CMakeLists.txt 必须包含：

CMake

cmake_minimum_required(VERSION 3.10)
project(yaml4c C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(YAML REQUIRED yaml-0.1)

add_library(yaml4c SHARED src/yaml4c.c)
target_include_directories(yaml4c PUBLIC include)
target_link_libraries(yaml4c ${YAML_LIBRARIES})

# Example Executable
add_executable(demo examples/main.c)
target_link_libraries(demo yaml4c)
6. 代码风格 (Style Guide)
命名：全小写 + 下划线 (y4c_load_file)。

缩进：4 空格。

注释：关键逻辑必须写注释（英文或中文皆可，保持统一）。

文件结构：

src/ : .c 源文件

include/ : 公共头文件

examples/ : 示例代码