#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>

#include <nlohmann/json.hpp>
#include <nlohmann/json-schema.hpp>

#include <SPIRV/GlslangToSpv.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <spirv_cross/spirv_cross.hpp>
#include <utility>

#include "schema.h"

// format, read_dir, write_dir, filepaths, filenames, shadercode, existent
typedef struct {
    std::string format;
    std::string read_dir;
    std::string write_dir;
    std::vector<std::string> filepaths;
    std::vector<std::string> filenames;
    std::vector<std::vector<unsigned int> > shadercode;
    EShLanguage stage;
    bool existent = true;
} shadertype_info;

std::string shaderstages[] = {"frag"}; // , "vert", "comp"

std::map<std::string, shadertype_info> shader_info;

// std::string frag_read_directory;
// std::string vert_read_directory;
// std::string comp_read_directory;
// std::string frag_write_directory;
// std::string vert_write_directory;
// std::string comp_write_directory;

// std::vector<std::string> frag_filepaths;
// std::vector<std::string> vert_filepaths;
// std::vector<std::string> comp_filepaths;
// std::vector<std::string> frag_filenames;
// std::vector<std::string> vert_filenames;
// std::vector<std::string> comp_filenames;

// std::vector<std::vector<unsigned int> > spirv_frag_shadercode;
// std::vector<std::vector<unsigned int> > spirv_vert_shadercode;
// std::vector<std::vector<unsigned int> > spirv_comp_shadercode;

extern const char * helpMessage;
void exitmsg(int code, const char * message);
void exitmsg(int code, std::string message);
std::string glsl_shader_stage(std::string filename);
std::string hlsl_shader_stage(std::string filename);
std::string spv_shader_stage(std::string filename);
std::string shader_stage(std::string filename, std::string read_format);

int main(int argc, char * argv[]) {

    shader_info["frag"].stage = EShLangFragment;
    shader_info["vert"].stage = EShLangVertex;
    shader_info["comp"].stage = EShLangCompute;

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
                shader_info["frag"].read_dir = compile["shaders"]["read_folder"];
                shader_info["vert"].read_dir = shader_info["frag"].read_dir;
                shader_info["comp"].read_dir = shader_info["frag"].read_dir;
                if (!std::filesystem::is_directory(compile["shaders"]["write_folder"])) exitmsg(1, "Error: the directory specified in write_folder is invalid!");
                shader_info["frag"].write_dir = compile["shaders"]["write_folder"];
                shader_info["vert"].write_dir = shader_info["frag"].write_dir;
                shader_info["comp"].write_dir = shader_info["frag"].write_dir;
                if (compile["shaders"].contains("files")) {
                    for (int i = 0; i < compile["shaders"]["files"].size(); i++){
                        std::string filepath = std::string(compile["shaders"]["read_folder"]) + "/" + std::string(compile["shaders"]["files"][i]);
                        if (!std::filesystem::exists(filepath)) exitmsg(1, std::string("Error: the file ") + std::string(compile["shaders"]["files"][i]) + " cannot be found!");
                        if (shader_stage(filepath, compile["read_format"]) != ""){
                            shader_info[shader_stage(filepath, compile["read_format"])].filepaths.push_back(filepath);
                            shader_info[shader_stage(filepath, compile["read_format"])].filenames.push_back(compile["shaders"]["files"][i]);
                        }
                        else exitmsg(1, std::string("Error: the file ") + std::string(compile["shaders"]["files"][i]) + " does not exist!");
                    }
                } else {
                    for (const auto& entry: std::filesystem::directory_iterator(compile["shaders"]["frag"]["read_folder"])) { // TODO: FIX
                        if (shader_stage(entry.path(), compile["read_format"]) != ""){
                            shader_info[shader_stage(entry.path(), compile["read_format"])].filepaths.push_back(entry.path());
                            shader_info[shader_stage(entry.path(), compile["read_format"])].filenames.push_back(entry.path().filename());
                        } else {
                            exitmsg(1, std::string("Error: the file ") + std::string(entry.path()) + " cannot be identified!"); // TODO: turn into warning/info
                        }
                    }
                }


            } else { // Check for standard (nonlazy)
                // Check shaderstages
                for (int i = 0; i < sizeof(shaderstages)/sizeof(shaderstages[0]); i++){
                    if (compile["shaders"].contains(shaderstages[i])) { // TODO: fix errors/warnings saying 'frag'
                        if (!std::filesystem::is_directory(std::string(compile["shaders"][shaderstages[i]]["read_folder"]))) exitmsg(1, "Error: the directory specified for frag in read_folder is invalid!");
                        shader_info[shaderstages[i]].read_dir = compile["shaders"][shaderstages[i]]["read_folder"];
                        if (!std::filesystem::is_directory(std::string(compile["shaders"][shaderstages[i]]["write_folder"]))) exitmsg(1, "Error: the directory specified for frag in write_folder is invalid!");
                        shader_info[shaderstages[i]].write_dir = compile["shaders"][shaderstages[i]]["write_folder"];
                        if (compile["shaders"][shaderstages[i]].contains("files")){
                            if (compile["shaders"][shaderstages[i]]["files"].size() == 0) {
                                std::cout << "Warning: 'files' is defined in 'frag', but it is empty!";
                                shader_info[shaderstages[i]].existent = false;
                            }
                            for (int i = 0; i < compile["shaders"][shaderstages[i]]["files"].size(); i++){
                                if (std::filesystem::exists(std::string(compile["shaders"][shaderstages[i]]["read_folder"]) + "/" + std::string(compile["shaders"][shaderstages[i]]["files"][i]))){
                                    shader_info[shaderstages[i]].filepaths.push_back(std::string(compile["shaders"][shaderstages[i]]["read_folder"]) + "/" + std::string(compile["shaders"][shaderstages[i]]["files"][i]));
                                    shader_info[shaderstages[i]].filenames.push_back(std::string(compile["shaders"][shaderstages[i]]["files"][i]));
                                }
                                else exitmsg(1, std::string("Error: for frag, the file ") + std::string(compile["shaders"][shaderstages[i]]["files"][i]) + " cannot be found!");
                            }
                        } else {
                            for (const auto& entry: std::filesystem::directory_iterator(compile["shaders"][shaderstages[i]]["read_folder"])) {
                                if (shader_stage(entry.path(), std::string(compile["read_format"])) == shaderstages[i]){
                                    shader_info[shaderstages[i]].filepaths.push_back(entry.path());
                                    shader_info[shaderstages[i]].filenames.push_back(entry.path().filename());
                                }
                            }
                        }
                    } else {
                        // No shaders needing compilation!
                        shader_info[shaderstages[i]].existent = false;
                    }
                }
            }
            // TODO: Check access for folders and files
            // TODO: Check if shader files are empty


            // Compile stuff
            if (compile["read_format"] != "spirv"){
                glslang::InitializeProcess();
                glslang::EShSource read_source;
                if (compile["read_format"] == "glsl") read_source = glslang::EShSourceGlsl;
                // glslangValidator -> SPIRV
                //
                // Compile to SPIRV here

                // Compile fragment shaders
                for (int i = 0; i < sizeof(shaderstages)/sizeof(shaderstages[0]); i++){
                    for (int i = 0; i < shader_info[shaderstages[i]].filepaths.size(); i++){
                        std::ifstream prefile(shader_info[shaderstages[i]].filepaths[i]);
                        std::string file_string((std::istreambuf_iterator<char>(prefile)), std::istreambuf_iterator<char>());
                        const char * file_code = file_string.c_str();

                        glslang::TShader shader(shader_info[shaderstages[i]].stage);
                        shader.setStrings(&file_code, 1);
                        shader.setEnvInput(read_source, shader_info[shaderstages[i]].stage, glslang::EShClientVulkan, 450);
                        shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
                        shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

                        TBuiltInResource resources = *GetDefaultResources();
                        if (!shader.parse(&resources, 450, false, EShMsgDefault)) {
                            exitmsg(1, std::string("Error: shader file ") + shader_info[shaderstages[i]].filepaths[i] + " is invalid!\n" + shader.getInfoLog());
                        }
                        glslang::TProgram program;
                        program.addShader(&shader);

                        if (!program.link(EShMsgDefault)) {
                            exitmsg(1, std::string("Error: shader file ") + shader_info[shaderstages[i]].filepaths[i] + " is invalid!\n" + program.getInfoLog());
                        }

                        glslang::TIntermediate * intermediate = program.getIntermediate(shader_info[shaderstages[i]].stage);
                        if (!intermediate) {
                            exitmsg(1, std::string("Error: intermediate failed!\n") + shader_info[shaderstages[i]].filepaths[i] + "\n" + shader.getInfoLog());
                        }

                        std::vector<unsigned int> spirv;
                        glslang::GlslangToSpv(*intermediate, spirv);
                        shader_info[shaderstages[i]].shadercode.push_back(spirv);
                    }
                }


                if (compile["write_format"] == "spirv"){
                    for (int i = 0; i < sizeof(shaderstages)/sizeof(shaderstages[0]); i++){
                        for (int i = 0; i < shader_info[shaderstages[i]].shadercode.size(); i++){
                            std::ofstream out_file(shader_info[shaderstages[i]].write_dir + "/" + shader_info[shaderstages[i]].filenames[i] + ".spv");
                            if (out_file) {
                                size_t size = shader_info[shaderstages[i]].shadercode.size();
                                out_file.write(reinterpret_cast<const char*>(shader_info[shaderstages[i]].shadercode.data()), size * sizeof(int));
                                out_file.close();
                            }
                            else {
                                exitmsg(1, std::string("Error: file ") + shader_info[shaderstages[i]].filepaths[i] + " is invalid!\n"); // TODO: Handle error properly
                            }
                        }
                    }
                } else {
                    // current (SPIRV) -> spirv-shadercross
                    //
                    // Compile to requested format here
                }


                glslang::FinalizeProcess();
            } else {
                // SPIRV -> write_format
            }
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

std::string shader_stage(std::string filename, std::string read_format) {
    if (read_format == "glsl") return glsl_shader_stage(filename);
    if (read_format == "hlsl") return hlsl_shader_stage(filename);
    if (read_format == "spv") return spv_shader_stage(filename);

    return "";
}

std::string glsl_shader_stage(std::string filename) {
    if (filename.ends_with(".frag")) return "frag";
    if (filename.ends_with(".vert")) return "vert";
    if (filename.ends_with(".comp")) return "comp";

    return "";
}

std::string hlsl_shader_stage(std::string filename) {
    if (filename.ends_with(".frag.hlsl")) return "frag";
    if (filename.ends_with(".vert.hlsl")) return "vert";
    if (filename.ends_with(".comp.hlsl")) return "comp";

    return "";
}

std::string spv_shader_stage(std::string filename) { // TODO: Make SPIRV input work without preextension
    if (filename.ends_with(".frag.spv")) return "frag";
    if (filename.ends_with(".vert.spv")) return "vert";
    if (filename.ends_with(".comp.spv")) return "comp";

    return "";
}
