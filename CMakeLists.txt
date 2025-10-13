cmake_minimum_required(VERSION 3.30)
project(monosc)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # TIME SAVER
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 23)

add_executable(monosc src/main.cpp)

target_include_directories(monosc PRIVATE /usr/local/include)
set_target_properties(monosc PROPERTIES RUNTIME_OUTPUT_DIRECTORY "monosc")

add_subdirectory(external/json)
add_subdirectory(external/json-schema-validator)

add_subdirectory(external/glslang)
add_subdirectory(external/SPIRV-Cross)

target_link_libraries(monosc PRIVATE glslang::glslang glslang::SPIRV spirv-cross-core spirv-cross-glsl) # Link glslang and SPIRV-Cross
target_link_libraries(monosc PRIVATE nlohmann_json::nlohmann_json nlohmann_json_schema_validator::validator) # Link nlohmann-json and schema validator

target_include_directories(monosc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/glslang ${CMAKE_CURRENT_SOURCE_DIR}/external/spirv-cross)
