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
            // Set CWD
            if (!std::filesystem::is_directory(argv[1])) exitmsg(1, "Error: the directory you provided is invalid!");
            std::filesystem::current_path(argv[1]);

            // Verify json file existence
            if (!std::filesystem::is_regular_file("shader_compile.json")) exitmsg(1, "Error: shader_compile.json could not be found at the path specified");

            // initialize shader_compile.json schema
            nlohmann::json schema = nlohmann::json::parse(shader_compile_schema);
            nlohmann::json_schema::json_validator val;
            val.set_root_schema(schema);

            nlohmann::json compile;

            // Check if shader_compile.json is valid json
            try {
                compile = nlohmann::json::parse(std::ifstream("shader_compile.json"));
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
                if (!std::filesystem::is_directory(compile["shaders"]["read_folder"])) exitmsg(1, "Error: the directory specified in read_folder is invalid!");
                if (!std::filesystem::is_directory(compile["shaders"]["write_folder"])) exitmsg(1, "Error: the directory specified in write_folder is invalid!");
                for (int i = 0; i < compile["shaders"]["files"].size(); i++){
                    if (!std::filesystem::exists(std::string(compile["shaders"]["read_folder"]) + "/" + std::string(compile["shaders"]["files"][i]))) exitmsg(1, std::string("Error: the file ") + std::string(compile["shaders"]["files"][i]) + " cannot be found!");
                }
            } else { // Check for standard
                // Check frag
                if (!std::filesystem::is_directory(std::string(compile["shaders"]["frag"]["read_folder"]))) exitmsg(1, "Error: the directory specified for frag in read_folder is invalid!");
                if (!std::filesystem::is_directory(std::string(compile["shaders"]["frag"]["write_folder"]))) exitmsg(1, "Error: the directory specified for frag in write_folder is invalid!");
                for (int i = 0; i < compile["shaders"]["frag"]["files"].size(); i++){
                    if (!std::filesystem::exists(std::string(compile["shaders"]["frag"]["read_folder"]) + "/" + std::string(compile["shaders"]["frag"]["files"][i]))) exitmsg(1, std::string("Error: for frag, the file ") + std::string(compile["shaders"]["frag"]["files"][i]) + " cannot be found!");
                }
                // Check vert
                if (!std::filesystem::is_directory(std::string(compile["shaders"]["vert"]["read_folder"]))) exitmsg(1, "Error: the directory specified for vert in read_folder is invalid!");
                if (!std::filesystem::is_directory(std::string(compile["shaders"]["vert"]["write_folder"]))) exitmsg(1, "Error: the directory specified for vert in write_folder is invalid!");
                for (int i = 0; i < compile["shaders"]["vert"]["files"].size(); i++){
                    if (!std::filesystem::exists(std::string(compile["shaders"]["vert"]["read_folder"]) + "/" + std::string(compile["shaders"]["vert"]["files"][i]))) exitmsg(1, std::string("Error: for vert, the file ") + std::string(compile["shaders"]["vert"]["files"][i]) + " cannot be found!");
                }
                if (!std::filesystem::is_directory(std::string(compile["shaders"]["comp"]["read_folder"]))) exitmsg(1, "Error: the directory specified for comp in read_folder is invalid!");
                if (!std::filesystem::is_directory(std::string(compile["shaders"]["comp"]["write_folder"]))) exitmsg(1, "Error: the directory specified for comp in write_folder is invalid!");
                for (int i = 0; i < compile["shaders"]["comp"]["files"].size(); i++){
                    if (!std::filesystem::exists(std::string(compile["shaders"]["comp"]["read_folder"]) + "/" + std::string(compile["shaders"]["comp"]["files"][i]))) exitmsg(1, std::string("Error: for comp, the file ") + std::string(compile["shaders"]["comp"]["files"][i]) + " cannot be found!");
                }
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
