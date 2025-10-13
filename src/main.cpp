#include <iostream>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <nlohmann/json-schema.hpp>
#include <string>
#include "schema.h"

extern const char * helpMessage;
void exitmsg(int code, const char * message);
void exitmsg(int code, std::string message);

int main(int argc, char * argv[]) {

    switch (argc) {
        case 1:
            std::cout << helpMessage;
            break;
        case 2:
            // Verify json file existence
            std::string shader_compile_path = std::string(argv[1]) + "/" + std::string("shader_compile.json");
            if (!std::filesystem::is_regular_file(shader_compile_path)) exitmsg(1, "Error: shader_compile.json could not be found at the path specified");

            // initialize shader_compile.json schema
            nlohmann::json schema = nlohmann::json::parse(shader_compile_schema);
            nlohmann::json_schema::json_validator val;
            val.set_root_schema(schema);

            nlohmann::json compile;

            // Check if shader_compile.json is valid json
            try {
                compile = nlohmann::json::parse(std::ifstream(shader_compile_path));
            } catch (const std::exception &e) {
                exitmsg(1, std::string("Error: shader_compile.json is invalid.\n") + e.what());
            }

            // Check if shader_compile.json follows schema
            try {
                val.validate(compile);
            } catch (const std::exception &e) {
                exitmsg(1, std::string("Error: shader_compile.json did not pass schema.\n") + std::string(e.what()));
            }

            // Prevent read and write formats from being identical
            if (compile["read_format"] == compile["write_format"]) exitmsg(1, "Error: the read_format and write_format cannot be identical!");

            // Check if file paths are valid
            if (compile["lazy"]) {
                if (!std::filesystem::is_directory(std::string(argv[1]) + "/" + std::string(compile["shaders"]["read_folder"]))) exitmsg(1, "Error: the directory specified in read_folder is invalid!");
                if (!std::filesystem::is_directory(std::string(argv[1]) + "/" + std::string(compile["shaders"]["write_folder"]))) exitmsg(1, "Error: the directory specified in write_folder is invalid!");
                for (int i = 0; i < compile["shaders"]["files"].size(); i++){
                    if (!std::filesystem::exists(std::string(argv[1]) + "/" + std::string(compile["shaders"]["read_folder"]) + "/" + std::string(compile["shaders"]["files"][i]))) exitmsg(1, std::string("Error: the file ") + std::string(compile["shaders"]["files"][i]) + " cannot be found!");
                }
            } else { // Check for standard

            }

            //      TODO:
            //      Do stuff based on shader_compile.json
    }

    return 0;
}

const char * helpMessage =
R"(monosc <directory> [options]

parameters:
  <directory>         : The directory that houses your
                        shader_compile.json

options:
  --guess-hlsl-files  : Since HLSL does not have a strict naming
                        scheme, this option will guess what each
                        .hlsl file does. If you exclude this
                        option, you must prefix your file
                        extension with the shader type.

                        (.vert.hlsl, .frag.hlsl, or .comp.hlsl)

                        This option only works with "lazy": true
)";

void exitmsg(int code, const char * message){
    std::cout << message << std::endl;
    exit(code);
}

void exitmsg(int code, std::string message){
    std::cout << message << std::endl;
    exit(code);
}
