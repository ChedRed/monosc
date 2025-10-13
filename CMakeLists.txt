cmake_minimum_required(VERSION 3.30)
project(monosc)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # TIME SAVER
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 23)



add_executable(monosc src/main.cpp)
target_include_directories(monosc PRIVATE /usr/local/include)
set_target_properties(monosc PROPERTIES RUNTIME_OUTPUT_DIRECTORY "monosc")


find_package(nlohmann_json REQUIRED)
find_library(JSON_SCHEMA_VALIDATOR_LIBRARY
  NAMES nlohmann_json_schema_validator
  PATHS /usr/local/lib
  NO_DEFAULT_PATH)

target_link_libraries(monosc PRIVATE nlohmann_json::nlohmann_json ${JSON_SCHEMA_VALIDATOR_LIBRARY})
