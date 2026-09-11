cmake_minimum_required(VERSION 3.16)
project(luau_cpp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(external/luau)

add_executable(luau_cpp src/main.cpp)

target_link_libraries(luau_cpp PRIVATE
    Luau.Compiler
    Luau.VM
)

# preprocessor define for script directory
target_compile_definitions(luau_cpp PRIVATE
    SCRIPTS_DIR="${CMAKE_SOURCE_DIR}/scripts"
)

# Copy the scripts folder next to the built executable so relative paths still work
add_custom_command(TARGET luau_cpp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/scripts
        $<TARGET_FILE_DIR:luau_cpp>/scripts
)