cmake_minimum_required(VERSION 3.10)
project(yaml4c C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Find libyaml via PkgConfig
find_package(PkgConfig REQUIRED)
pkg_check_modules(YAML REQUIRED yaml-0.1)

# Main Library
add_library(yaml4c SHARED src/yaml4c.c)
target_include_directories(yaml4c PUBLIC include)
target_include_directories(yaml4c PRIVATE ${YAML_INCLUDE_DIRS})
target_link_directories(yaml4c PUBLIC ${YAML_LIBRARY_DIRS})
target_link_libraries(yaml4c ${YAML_LIBRARIES})

# Example Executable
add_executable(demo examples/main.c)
target_link_libraries(demo yaml4c)
