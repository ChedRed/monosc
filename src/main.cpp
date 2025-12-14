#include <string>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <nlohmann/json-schema.hpp>

#include "schema.hpp"
#include "glslcomp.hpp"
#include "spvcomp.hpp"

// format, read_dir, write_dir, filepaths, filenames, shadercode, existent
typedef struct {
    std::string format;
    std::string read_dir;
    std::string write_dir;
    std::vector<std::string> filepaths;
    std::vector<std::string> filenames;
    std::vector<std::string> entrypoints;
    EShLanguage stage;
    bool existent;
} shadertype_info;

const std::string shader_stages[] = {"frag", "vert", "comp"};

std::vector<std::string> temp_files;
std::map<std::string, shadertype_info> shader_info;

extern const char * helpMessage;
void exitmsg(int code, const char * message);
void exitmsg(int code, std::string message);
std::string glsl_shader_stage(std::string filename);
std::string hlsl_shader_stage(std::string filename);
std::string spv_shader_stage(std::string filename);
std::string shader_stage(std::string filename, std::string read_format);

int main(int argc, char * argv[]) {

    shader_info["frag"].stage = EShLangFragment;
    shader_info["frag"].existent = false;
    shader_info["vert"].stage = EShLangVertex;
    shader_info["vert"].existent = false;
    shader_info["comp"].stage = EShLangCompute;
    shader_info["comp"].existent = false;

    if (argc == 1){
        std::cout << helpMessage;
        return 0;
    }
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

    // Prevent read and write formats from being identical //Nah
    // if (compile["read_format"] == compile["write_format"]) exitmsg(1, "Error: the read_format and write_format cannot be identical!");

    // Check if file paths are valid
    if (compile["lazy"]) {
        if (!std::filesystem::is_directory(compile["shaders"]["read_folder"])) exitmsg(1, "Error: the directory specified in read_folder is invalid!");
        if (!std::filesystem::is_directory(compile["shaders"]["write_folder"])) exitmsg(1, "Error: the directory specified in write_folder is invalid!");
        for (const auto& stage: shader_stages){
            shader_info[stage].read_dir = compile["shaders"]["read_folder"];
            shader_info[stage].write_dir = compile["shaders"]["write_folder"];
        }
        if (compile["shaders"].contains("files")) {
            for (int i = 0; i < (int)compile["shaders"]["files"].size(); i++){
                std::string filepath = std::string(compile["shaders"]["read_folder"]) + "/" + std::string(compile["shaders"]["files"][i]);
                std::cout << filepath << std::endl;
                if (!std::filesystem::exists(filepath)) exitmsg(1, std::string("Error: the file ") + std::string(compile["shaders"]["files"][i]) + " cannot be found!");
                if (shader_stage(filepath, compile["read_format"]) != ""){
                    shader_info[shader_stage(filepath, compile["read_format"])].existent = true;
                    shader_info[shader_stage(filepath, compile["read_format"])].filepaths.push_back(filepath);
                    shader_info[shader_stage(filepath, compile["read_format"])].filenames.push_back(compile["shaders"]["files"][i]);
                    if (compile["shaders"].contains("entrypoints")){
                        if (compile["shaders"]["entrypoints"].size() == compile["shaders"]["files"].size()){
                            shader_info[shader_stage(filepath, compile["read_format"])].entrypoints.push_back(compile["shaders"]["entrypoints"][i]);
                            std::cout << shader_stage(filepath, compile["read_format"]) << "\n" << compile["shaders"]["entrypoints"][i] << std::endl;
                        }
                    }
                    std::cout << compile["shaders"]["files"][i] << std::endl;
                }
                else exitmsg(1, std::string("Error: the file ") + std::string(compile["shaders"]["files"][i]) + " does not exist!");
            }
        } else {
            for (const auto& entry: std::filesystem::directory_iterator(compile["shaders"]["read_folder"])) {
                if (shader_info[shader_stage(entry.path(), compile["read_format"])].filepaths.empty()) continue;
                shader_info[shader_stage(entry.path(), compile["read_format"])].existent = true;
                if (shader_stage(entry.path(), compile["read_format"]) != ""){
                    shader_info[shader_stage(entry.path(), compile["read_format"])].filepaths.push_back(entry.path());
                    shader_info[shader_stage(entry.path(), compile["read_format"])].filenames.push_back(entry.path().filename());
                } else {
                    std::cout << "INFO: The item " << entry.path() << " cannot be identified, and was skipped.\n";
                }
            }

        }


    } else { // Check for standard (nonlazy)
        for (const auto& stage: shader_stages){
            if (compile["shaders"].contains(stage)) {
                shader_info[stage].existent = true;
                if (!std::filesystem::is_directory(std::string(compile["shaders"][stage]["read_folder"]))) exitmsg(1, std::string("Error: the directory specified for ") + stage + " in read_folder is invalid!");
                shader_info[stage].read_dir = compile["shaders"][stage]["read_folder"];
                if (!std::filesystem::is_directory(std::string(compile["shaders"][stage]["write_folder"]))) exitmsg(1, std::string("Error: the directory specified for ") + stage + " in write_folder is invalid!");
                shader_info[stage].write_dir = compile["shaders"][stage]["write_folder"];
                if (compile["shaders"][stage].contains("files")){
                    if (compile["shaders"][stage]["files"].size() == 0) {
                        std::cout << "Warning: 'files' is defined in '" << stage << "', but it is empty!";
                        shader_info[stage].existent = false;
                    }
                    for (int i = 0; i < (int)compile["shaders"][stage]["files"].size(); i++){
                        if (std::filesystem::exists(std::string(compile["shaders"][stage]["read_folder"]) + "/" + std::string(compile["shaders"][stage]["files"][i]))){
                            shader_info[stage].filepaths.push_back(std::string(compile["shaders"][stage]["read_folder"]) + "/" + std::string(compile["shaders"][stage]["files"][i]));
                            shader_info[stage].filenames.push_back(std::string(compile["shaders"][stage]["files"][i]));
                            if (compile["shaders"].contains("entrypoints")){
                                if (compile["shaders"]["entrypoints"].size() == compile["shaders"]["files"].size()){
                                    shader_info[stage].entrypoints.push_back(compile["shaders"]["entrypoints"][i]);
                                }
                            }
                        }
                        else exitmsg(1, std::string("Error: for") +  stage + ", the file " + std::string(compile["shaders"][stage]["files"][i]) + " cannot be found!");
                    }
                } else {
                    for (const auto& entry: std::filesystem::directory_iterator(compile["shaders"][stage]["read_folder"])) {
                        if (shader_stage(entry.path(), std::string(compile["read_format"])) == stage){
                            std::cout << "Info: adding file " << entry.path() << std::endl;
                            shader_info[stage].filepaths.push_back(entry.path());
                            shader_info[stage].filenames.push_back(entry.path().filename());
                        }
                    }
                    if (shader_info[stage].filepaths.empty()) shader_info[stage].existent = false;
                }
            }
        }
    }

    // Compile stuff 2.0
    for (const auto& stage: shader_stages){
        if (!shader_info[stage].existent){
            std::cout << "Skipped " << stage << std::endl;
            continue;
        }
        for (int i = 0; i < (int)shader_info[stage].filepaths.size(); i++){
            bool ms_bs = compile["write_format"] == "dxbc" || compile["write_format"] == "dxil";
            std::vector<uint32_t> spv_shadercode;

            if (compile["read_format"] == "spirv") {
                std::ifstream prefile(shader_info[stage].filepaths[i]);
                // Put prefile into spv_shadercode
            } else {
                if (compile["read_format"] == "hlsl" && ms_bs){
                    // Start dxc
                    // Compile direct DXBC/DXIL and END
                }

                glslangwrap glslwrap = glslangwrap();
                glslang::EShSource read_source = glslang::EShSourceHlsl;
                if (compile["read_format"] == "glsl") read_source = glslang::EShSourceGlsl;
                glslwrap.compileglsl(shader_info[stage].filepaths[i].c_str(), spv_shadercode, read_source, shader_info[stage].stage);
                glslwrap.destroy();
            }


            if (compile["write_format"] == "spirv"){
                std::ofstream out_file(shader_info[stage].write_dir + "/" + shader_info[stage].filenames[i] + ".spv");
                if (out_file) {
                    out_file.write(reinterpret_cast<const char*>(spv_shadercode.data()), spv_shadercode.size() * sizeof(uint32_t));
                    out_file.close();
                }
                else {
                    std::cout << "WARN: A file at the directory below cannot be created. Are you sure you have permission to write to this directory?\n" << shader_info[stage].write_dir << "\n" << shader_info[stage].write_dir + "/" + shader_info[stage].filenames[i] + ".spv\n";
                }
            } else {
                if  (ms_bs){
                    // Compile HLSL -> DXBC/DXIL
                    exitmsg(1, "dxc is not supported yet!");
                } else {
                    std::string unextension = shader_info[stage].filenames[i];
                    if (compile["read_format"] != "glsl") {
                        int extension_length = std::string(compile["read_format"]).length() + 1;
                        unextension.erase(unextension.size()-extension_length, extension_length);
                    }


                    std::string extension = "";
                    if (compile["write_format"] != "glsl") extension = "." + std::string(compile["write_format"]);
                    std::ofstream out_file;
                    if (compile["write_format"] == "metallib"){
                        out_file.open((std::string)std::filesystem::temp_directory_path() + unextension + ".metal");
                        temp_files.push_back((std::string)std::filesystem::temp_directory_path() + unextension + ".metal");
                    } else{
                        out_file.open(shader_info[stage].write_dir + "/" + unextension + extension);
                    }


                    if (out_file) {
                        std::string returnv;
                        if (compile["write_format"] == "metallib"){
                            std::cout << shader_info[stage].entrypoints[i] << std::endl;
                            returnv = compilespv(spv_shadercode, compile["write_format"], shader_info[stage].entrypoints[i], stage);
                        } else {
                            returnv = compilespv(spv_shadercode, compile["write_format"]);
                        }
                        out_file.write(returnv.c_str(), returnv.length());
                        out_file.close();
                    }
                    else {
                        std::cout << "WARN: A file at the directory below cannot be created. Are you sure you have permission to write to this directory?\n" << shader_info[stage].write_dir << "\n" << shader_info[stage].write_dir + "/" + shader_info[stage].filenames[i] + ".spv";
                    }
                }
            }
        }
    }
    if (compile["write_format"] == "metallib"){
        // Compile all created .metal files in the temp dir into one .metallib
        std::string rawfiles = "xcrun -sdk macosx metal ";
        for (auto& entry : temp_files) {
            rawfiles.append(entry + " ");
        }
        rawfiles.append("-o " + shader_info[shader_stages[0]].write_dir + "/default.metallib");
        system(rawfiles.c_str());
        std::cout << rawfiles << std::endl;

    }
    return 0;
}

const char * helpMessage =
R"(monosc <directory>

parameters:
  <directory>         : The directory that houses your
                        shader_compile.json
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
