cmake_minimum_required(VERSION 3.30)
project(monosc LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Force consistent compile flags globally
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Werror -Wno-deprecated-copy)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_compile_options(-stdlib=libc++)
    endif()
endif()


# Optionally disable exceptions in SPIRV-Cross to avoid ABI issues
set(SPIRV_CROSS_STATIC ON CACHE BOOL "Build SPIRV-Cross static lib")
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "Disable SPIRV-Cross tests")
set(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS ON CACHE BOOL "Disable exceptions in SPIRV-Cross")

add_subdirectory(external/json)
add_subdirectory(external/json-schema-validator)
add_subdirectory(external/glslang)
add_subdirectory(external/SPIRV-Cross)

add_executable(monosc
    src/main.cpp
    src/glslcomp.cpp
    src/spvcomp.cpp
)

target_include_directories(monosc PRIVATE /usr/local/include)
target_include_directories(monosc PRIVATE external/SPIRV-Cross external/glslang)
target_sources(monosc PRIVATE external/glslang/glslang/ResourceLimits/ResourceLimits.cpp)

# Linking dependencies
target_link_libraries(monosc PRIVATE
    glslang::glslang
    glslang::SPIRV
    spirv-cross-core
    spirv-cross-cpp
    spirv-cross-glsl
    spirv-cross-hlsl
    spirv-cross-msl
    nlohmann_json::nlohmann_json
    nlohmann_json_schema_validator::validator
)

set_target_properties(monosc PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/monosc")
