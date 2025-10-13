#include <_types/_uint32_t.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <regex>
#include <nlohmann/json.hpp>
#include <nlohmann/json-schema.hpp>
#include <glslang/Public/ShaderLang.h>
#include <spirv_cross/spirv_cross.hpp>
#include <string>

#include "schema.h"

bool existent_shader_types[3] = { true, true, true };

std::string frag_directory;
std::string vert_directory;
std::string comp_directory;

std::vector<std::string> frag_filepaths;
std::vector<std::string> vert_filepaths;
std::vector<std::string> comp_filepaths;

extern const char * helpMessage;
void exitmsg(int code, const char * message);
void exitmsg(int code, std::string message);
uint32_t glsl_shader_stage(std::string filename);
uint32_t hlsl_shader_stage(std::string filename);
uint32_t spv_shader_stage(std::string filename);
uint32_t shader_stage(std::string filename, std::string read_format);

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
                frag_directory = compile["shaders"]["read_folder"];
                vert_directory = frag_directory;
                comp_directory = frag_directory;
                if (!std::filesystem::is_directory(compile["shaders"]["write_folder"])) exitmsg(1, "Error: the directory specified in write_folder is invalid!");
                if (compile["shaders"].contains("files")) {
                    for (int i = 0; i < compile["shaders"]["files"].size(); i++){
                        std::string filepath = std::string(compile["shaders"]["read_folder"]) + "/" + std::string(compile["shaders"]["files"][i]);
                        if (!std::filesystem::exists(filepath)) exitmsg(1, std::string("Error: the file ") + std::string(compile["shaders"]["files"][i]) + " cannot be found!");
                        if (compile["read_format"] == "glsl"){ // Categorize files based on extension for GLSL
                            if (glsl_shader_stage(filepath) == 0) frag_filepaths.push_back(filepath);
                            else if (glsl_shader_stage(filepath) == 1) vert_filepaths.push_back(filepath);
                            else if (glsl_shader_stage(filepath) == 2) comp_filepaths.push_back(filepath);
                            else exitmsg(1, std::string("Error: the file ") + std::string(compile["shaders"]["files"][i]) + " does not exist!");
                        } else if (compile["read_format"] == "hlsl"){ // Categorize files based on preextension for HLSL
                            if (hlsl_shader_stage(filepath) == 0) frag_filepaths.push_back(filepath);
                            else if (hlsl_shader_stage(filepath) == 1) vert_filepaths.push_back(filepath);
                            else if (hlsl_shader_stage(filepath) == 2) comp_filepaths.push_back(filepath);
                            else exitmsg(1, std::string("Error: the file ") + std::string(compile["shaders"]["files"][i]) + " does not exist!");
                        } else if (compile["read_format"] == "spv"){ // Categorize files based on preextension for SPIRV
                            if (spv_shader_stage(filepath) == 0) frag_filepaths.push_back(filepath);
                            else if (spv_shader_stage(filepath) == 1) vert_filepaths.push_back(filepath);
                            else if (spv_shader_stage(filepath) == 2) comp_filepaths.push_back(filepath);
                            else exitmsg(1, std::string("Error: the file ") + std::string(compile["shaders"]["files"][i]) + " does not exist!");
                        }
                    }
                } else {
                    for (const auto& entry: std::filesystem::directory_iterator(compile["shaders"]["frag"]["read_folder"])) {
                        if (compile["read_format"] == "glsl"){ // Categorize files based on extension for GLSL
                            if (glsl_shader_stage(entry.path()) == 0) frag_filepaths.push_back(entry.path());
                            else if (glsl_shader_stage(entry.path()) == 1) vert_filepaths.push_back(entry.path());
                            else if (glsl_shader_stage(entry.path()) == 2) comp_filepaths.push_back(entry.path());
                        } else if (compile["read_format"] == "hlsl"){ // Categorize files based on preextension for HLSL
                            if (hlsl_shader_stage(entry.path()) == 0) frag_filepaths.push_back(entry.path());
                            else if (hlsl_shader_stage(entry.path()) == 1) vert_filepaths.push_back(entry.path());
                            else if (hlsl_shader_stage(entry.path()) == 2) comp_filepaths.push_back(entry.path());
                        } else if (compile["read_format"] == "spv"){ // Categorize files based on preextension for SPIRV
                            if (spv_shader_stage(entry.path()) == 0) frag_filepaths.push_back(entry.path());
                            else if (spv_shader_stage(entry.path()) == 1) vert_filepaths.push_back(entry.path());
                            else if (spv_shader_stage(entry.path()) == 2) comp_filepaths.push_back(entry.path());
                        }
                    }
                }


            } else { // Check for standard (nonlazy)
                // Check frag
                if (compile["shaders"].contains("frag")) {
                    if (!std::filesystem::is_directory(std::string(compile["shaders"]["frag"]["read_folder"]))) exitmsg(1, "Error: the directory specified for frag in read_folder is invalid!");
                    frag_directory = compile["shaders"]["frag"]["read_folder"];
                    if (!std::filesystem::is_directory(std::string(compile["shaders"]["frag"]["write_folder"]))) exitmsg(1, "Error: the directory specified for frag in write_folder is invalid!");
                    if (compile["shaders"]["frag"].contains("files")){
                        if (compile["shaders"]["frag"]["files"].size() == 0) {
                            std::cout << "Warning: 'files' is defined in 'frag', but it is empty!";
                        }
                        for (int i = 0; i < compile["shaders"]["frag"]["files"].size(); i++){
                            if (std::filesystem::exists(std::string(compile["shaders"]["frag"]["read_folder"]) + "/" + std::string(compile["shaders"]["frag"]["files"][i]))) frag_filepaths.push_back(std::string(compile["shaders"]["frag"]["read_folder"]) + "/" + std::string(compile["shaders"]["frag"]["files"][i]));
                            else exitmsg(1, std::string("Error: for frag, the file ") + std::string(compile["shaders"]["frag"]["files"][i]) + " cannot be found!");
                        }
                    } else {
                        for (const auto& entry: std::filesystem::directory_iterator(compile["shaders"]["frag"]["read_folder"])) {
                            if (shader_stage(entry.path(), std::string(compile["read_format"])) == 0) frag_filepaths.push_back(entry.path());
                        }
                    }

                } else {
                    // No fragment shaders needing comp!
                    existent_shader_types[0] = false;
                }

                // Check vert
                if (compile["shaders"].contains("vert")) {
                    if (!std::filesystem::is_directory(std::string(compile["shaders"]["vert"]["read_folder"]))) exitmsg(1, "Error: the directory specified for vert in read_folder is invalid!");
                    vert_directory = compile["shaders"]["vert"]["read_folder"];
                    if (!std::filesystem::is_directory(std::string(compile["shaders"]["vert"]["write_folder"]))) exitmsg(1, "Error: the directory specified for vert in write_folder is invalid!");
                    if (compile["shaders"]["vert"].contains("files")){
                        if (compile["shaders"]["vert"]["files"].size() == 0) {
                            std::cout << "Warning: 'files' is defined in 'frag', but it is empty!";
                        }
                        for (int i = 0; i < compile["shaders"]["vert"]["files"].size(); i++){
                            if (std::filesystem::exists(std::string(compile["shaders"]["vert"]["read_folder"]) + "/" + std::string(compile["shaders"]["vert"]["files"][i]))) vert_filepaths.push_back(std::string(compile["shaders"]["vert"]["read_folder"]) + "/" + std::string(compile["shaders"]["vert"]["files"][i]));
                            else exitmsg(1, std::string("Error: for vert, the file ") + std::string(compile["shaders"]["vert"]["files"][i]) + " cannot be found!");
                        }
                    } else {
                        for (const auto& entry: std::filesystem::directory_iterator(compile["shaders"]["vert"]["read_folder"])) {
                            if (shader_stage(entry.path(), std::string(compile["read_format"])) == 1) vert_filepaths.push_back(entry.path());
                        }
                    }
                } else {
                    // No vertex shaders needing comp!
                    existent_shader_types[1] = false;
                }

                if (compile["shaders"].contains("comp")) {
                    if (!std::filesystem::is_directory(std::string(compile["shaders"]["comp"]["read_folder"]))) exitmsg(1, "Error: the directory specified for comp in read_folder is invalid!");
                    comp_directory = compile["shaders"]["comp"]["read_folder"];
                    if (!std::filesystem::is_directory(std::string(compile["shaders"]["comp"]["write_folder"]))) exitmsg(1, "Error: the directory specified for comp in write_folder is invalid!");
                    if (compile["shaders"]["comp"].contains("files")){
                        if (compile["shaders"]["comp"]["files"].size() == 0) {
                            std::cout << "Warning: 'files' is defined in 'frag', but it is empty!";
                        }
                        for (int i = 0; i < compile["shaders"]["comp"]["files"].size(); i++){
                            if (std::filesystem::exists(std::string(compile["shaders"]["comp"]["read_folder"]) + "/" + std::string(compile["shaders"]["comp"]["files"][i]))) comp_filepaths.push_back(std::string(compile["shaders"]["comp"]["read_folder"]) + "/" + std::string(compile["shaders"]["comp"]["files"][i]));
                            else exitmsg(1, std::string("Error: for comp, the file ") + std::string(compile["shaders"]["comp"]["files"][i]) + " cannot be found!");
                        }
                    } else {
                        for (const auto& entry: std::filesystem::directory_iterator(compile["shaders"]["comp"]["read_folder"])) {
                            if (shader_stage(entry.path(), std::string(compile["read_format"])) == 2) comp_filepaths.push_back(entry.path());
                        }
                    }
                } else {
                    // No compute shaders needing comp!
                    existent_shader_types[2] = false;
                }
            }


            // Compile stuff
            if (compile["read_format"] != "spirv"){
                glslang::InitializeProcess();
                // glslangValidator -> SPIRV
                //
                // Compile to SPIRV here




                if (compile["write_format"] != "spirv"){
                    // current (SPIRV ) -> spirv-shadercross
                    //
                    // Compile to requested format here
                } else {

                }


                glslang::FinalizeProcess();
            } else {
                // SPIRV -> write_format
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

uint32_t shader_stage(std::string filename, std::string read_format) {
    if (read_format == "glsl") return glsl_shader_stage(filename);
    if (read_format == "hlsl") return hlsl_shader_stage(filename);
    if (read_format == "spv") return spv_shader_stage(filename);

    return -1;
}

uint32_t glsl_shader_stage(std::string filename) {
    if (filename.ends_with(".frag")) return 0;
    if (filename.ends_with(".vert")) return 1;
    if (filename.ends_with(".comp")) return 2;

    return -1;
}

uint32_t hlsl_shader_stage(std::string filename) {
    if (filename.ends_with(".frag.hlsl")) return 0;
    if (filename.ends_with(".vert.hlsl")) return 1;
    if (filename.ends_with(".comp.hlsl")) return 2;

    return -1;
}

uint32_t spv_shader_stage(std::string filename) { // TODO: Make SPIRV input work without preextension
    if (filename.ends_with(".frag.spv")) return 0;
    if (filename.ends_with(".vert.spv")) return 1;
    if (filename.ends_with(".comp.spv")) return 2;

    return -1;
}
