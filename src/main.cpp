#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>

#include <nlohmann/json.hpp>
#include <nlohmann/json-schema.hpp>
#include <utility>

#include "schema.h"
#include "glslcomp.h"
#include "spvcomp.h"

// format, read_dir, write_dir, filepaths, filenames, shadercode, existent
typedef struct {
    std::string format;
    std::string read_dir;
    std::string write_dir;
    std::vector<std::string> filepaths;
    std::vector<std::string> filenames;
    EShLanguage stage;
    bool existent = false;
} shadertype_info;

const std::string shader_stages[] = {"frag", "vert", "comp"};

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
                            std::cout << "WARN: A file at the directory below cannot be created. Are you sure you have permission to write to this directory?\n" << shader_info[stage].write_dir << "\n" << shader_info[stage].write_dir + "/" + shader_info[stage].filenames[i] + ".spv";
                        }
                    } else {
                        if  (ms_bs){
                            // Compile HLSL -> DXBC/DXIL
                        } else {
                            std::ofstream out_file(shader_info[stage].write_dir + "/" + shader_info[stage].filenames[i] + "." + std::string(compile["write_format"]));
                            if (out_file) {
                                std::string returnv = compilespv(spv_shadercode, compile["write_format"]);
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
            // Compile stuff
            // if (compile["read_format"] != "spirv"){
            //     glslang::InitializeProcess();
            //     glslang::EShSource read_source = glslang::EShSourceHLSL;
            //     if (compile["read_format"] == "glsl") read_source = glslang::EShSourceGlsl;

            //     // Compile shaders to SPIRV
            //     for (const auto& stage: shader_stages){
            //         if (!shader_info[stage].existent){
            //             std::cout << "Skipped " << stage << std::endl;
            //             continue;
            //         }
            //         for (int i = 0; i < shader_info[stage].filepaths.size(); i++){
            //             std::ifstream prefile(shader_info[stage].filepaths[i]);
            //             if (!prefile){
            //                 std::cout << "WARN: The file below was skipped. Are you sure you have permission to read this file?\n" << shader_info[stage].filepaths[i];
            //                 continue;
            //             }
            //             std::string file_string((std::istreambuf_iterator<char>(prefile)), std::istreambuf_iterator<char>());
            //             const char * file_code = file_string.c_str();

            //             glslang::TShader shader(shader_info[stage].stage);
            //             shader.setStrings(&file_code, 1);
            //             shader.setEnvInput(read_source, shader_info[stage].stage, glslang::EShClientVulkan, 450);
            //             shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2); // TODO: Make versions a config setting
            //             shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

            //             TBuiltInResource resources = *GetDefaultResources();
            //             if (!shader.parse(&resources, 450, false, EShMsgDefault)) {
            //                 exitmsg(1, std::string("Error: shader file ") + shader_info[stage].filepaths[i] + " is invalid!\n" + shader.getInfoLog());
            //             }
            //             glslang::TProgram program;
            //             program.addShader(&shader);

            //             if (!program.link(EShMsgDefault)) {
            //                 exitmsg(1, std::string("Error: shader file ") + shader_info[stage].filepaths[i] + " is invalid!\n" + program.getInfoLog());
            //             }

            //             glslang::TIntermediate * intermediate = program.getIntermediate(shader_info[stage].stage);
            //             if (!intermediate) {
            //                 exitmsg(1, std::string("Error: intermediate failed!\n") + shader_info[stage].filepaths[i] + "\n" + shader.getInfoLog());
            //             }

            //             std::vector<uint32_t> spirv;
            //             glslang::GlslangToSpv(*intermediate, spirv);
            //             // spirv_cross::CompilerHLSL hlsl(spirv.data(), spirv.size());
            //             std::cout << spirv.size() << std::endl;
            //             // shader_info[stage].shadercode.reserve(spirv.size());
            //             shader_info[stage].shadercode.push_back(spirv);
            //         }
            //     }

            //     glslang::FinalizeProcess();

                // Save SPIRV
                // if (compile["write_format"] == "spirv"){
                //     for (const auto& stage: shader_stages){
                //         for (int i = 0; i < shader_info[stage].shadercode.size(); i++){
                //             std::ofstream out_file(shader_info[stage].write_dir + "/" + shader_info[stage].filenames[i] + ".spv");
                //             if (out_file) {
                //                 size_t size = shader_info[stage].shadercode[i].size();
                //                 out_file.write(reinterpret_cast<const char*>(shader_info[stage].shadercode[i].data()), size * sizeof(int));
                //                 out_file.close();
                //             }
                //             else {
                //                 std::cout << "WARN: A file at the directory below cannot be created. Are you sure you have permission to write to this directory?\n" << shader_info[stage].write_dir << "\n" << shader_info[stage].write_dir + "/" + shader_info[stage].filenames[i] + ".spv";
                //             }
                //         }
                //     }
                // } else {
                //     // Compile existing SPIRV to other formats
                //     if (compile["write_format"] == "glsl"){ // TODO: simplify (nearly identical 3 things)
                //         for (const auto& stage: shader_stages){
                //             for (int i = 0; i < shader_info[stage].shadercode.size(); i++){
                //                 std::ofstream out_file(shader_info[stage].write_dir + "/" + shader_info[stage].filenames[i] + ".glsl");
                //                 if (out_file) {
                //                     spirv_cross::CompilerGLSL glsl(shader_info[stage].shadercode[i].data(), shader_info[stage].shadercode[i].size());
                //                    	spirv_cross::ShaderResources resources = glsl.get_shader_resources();

                //                    	for (auto &resource : resources.sampled_images){
                //                   		unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
                //                   		unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
                //                   		std::cout << "Image" << resource.name << "at set = " << set << ", binding = " << binding << std::endl;
                //                    	}

                //                    	spirv_cross::CompilerGLSL::Options options;
                //                    	options.version = 450;
                //                    	options.es = false; // TODO: Add to list of options in .json
                //                    	glsl.set_common_options(options);
                //                    	std::string source = glsl.compile();
                //                     std::cout << source << std::endl;
                //                     out_file << source;
                //                 }
                //                 else {
                //                     std::cout << "WARN: A file at the directory below cannot be created. Are you sure you have permission to write to this directory?\n" << shader_info[stage].write_dir << "\n" << shader_info[stage].write_dir + "/" + shader_info[stage].filenames[i] + ".spv";
                //                 }
                //             }
                //         }
                //     } else if (compile["write_format"] == "hlsl"){
                //         for (const auto& stage: shader_stages){
                //             for (int i = 0; i < shader_info[stage].shadercode.size(); i++){
                //                 std::ofstream out_file(shader_info[stage].write_dir + "/" + shader_info[stage].filenames[i] + ".hlsl");
                //                 if (out_file) {
                //                     std::cout << shader_info[stage].shadercode[i].data() << std::endl;
                //                     std::vector<uint32_t> spirv = shader_info[stage].shadercode[i];
                //                     spirv_cross::CompilerHLSL hlsl(spirv.data(), spirv.size());
                //                     std::cout << "pass" << std::endl;
                //                    	spirv_cross::ShaderResources resources = hlsl.get_shader_resources();

                //                    	for (auto& resource : resources.sampled_images){
                //                   		unsigned set = hlsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
                //                   		unsigned binding = hlsl.get_decoration(resource.id, spv::DecorationBinding);
                //                   		std::cout << "Image" << resource.name << "at set = " << set << ", binding = " << binding << std::endl;
                //                    	}

                //                    	spirv_cross::CompilerHLSL::Options options;
                //                    	options.shader_model = 50; // TODO: Add to list of options in .json
                //                    	hlsl.set_hlsl_options(options);
                //                    	std::string source = hlsl.compile();
                //                     std::cout << source << std::endl;
                //                     out_file << source;
                //                     // const SpvId * spirv = shader_info[stage].shadercode[i].data();
                //                     // size_t word_count = shader_info[stage].shadercode[i].size();

                //                     // spvc_context context = NULL;
                //                     // spvc_parsed_ir ir = NULL;
                //                     // spvc_compiler compiler_hlsl = NULL;
                //                     // spvc_compiler_options options = NULL;
                //                     // spvc_resources resources = NULL;
                //                     // const spvc_reflected_resource *list = NULL;
                //                     // const char *result = NULL;
                //                     // size_t count;
                //                     // size_t i;
                //                     // spvc_context_create(&context);

                //                     // // Set debug callback.
                //                     // // spvc_context_set_error_callback(context, error_callback, userdata);

                //                     // // Parse the SPIR-V.
                //                     // spvc_context_parse_spirv(context, spirv, word_count, &ir);

                //                     // // Hand it off to a compiler instance and give it ownership of the IR.
                //                     // spvc_context_create_compiler(context, SPVC_BACKEND_HLSL, ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler_hlsl);

                //                     // // Do some basic reflection.
                //                     // spvc_compiler_create_shader_resources(compiler_hlsl, &resources);
                //                     // spvc_resources_get_resource_list_for_type(resources, SPVC_RESOURCE_TYPE_UNIFORM_BUFFER, &list, &count);

                //                     // for (i = 0; i < count; i++)
                //                     // {
                //                     //     printf("ID: %u, BaseTypeID: %u, TypeID: %u, Name: %s\n", list[i].id, list[i].base_type_id, list[i].type_id,
                //                     //            list[i].name);
                //                     //     printf("  Set: %u, Binding: %u\n",
                //                     //            spvc_compiler_get_decoration(compiler_hlsl, list[i].id, SpvDecorationDescriptorSet),
                //                     //            spvc_compiler_get_decoration(compiler_hlsl, list[i].id, SpvDecorationBinding));
                //                     // }

                //                     // // Modify options.
                //                     // spvc_compiler_create_compiler_options(compiler_hlsl, &options);
                //                     // // spvc_compiler_options_set_uint(options, SPVC_COMPILER_OPTION_HLSL_, 330);
                //                     // spvc_compiler_install_compiler_options(compiler_hlsl, options);

                //                     // spvc_compiler_compile(compiler_hlsl, &result);
                //                     // printf("Cross-compiled source: %s\n", result);

                //                     // // Frees all memory we allocated so far.
                //                     // spvc_context_destroy(context);
                //                 }
                //                 else {
                //                     std::cout << "WARN: A file at the directory below cannot be created. Are you sure you have permission to write to this directory?\n" << shader_info[stage].write_dir << "\n" << shader_info[stage].write_dir + "/" + shader_info[stage].filenames[i] + ".spv";
                //                 }
                //             }
                //         }
                //     } else if (compile["write_format"] == "msl"){
                //         for (const auto& stage: shader_stages){
                //             for (int i = 0; i < shader_info[stage].shadercode.size(); i++){
                //                 std::ofstream out_file(shader_info[stage].write_dir + "/" + shader_info[stage].filenames[i] + ".hlsl");
                //                 if (out_file) {
                //                     spirv_cross::CompilerMSL msl(shader_info[stage].shadercode[i].data(), shader_info[stage].shadercode[i].size());
                //                    	spirv_cross::ShaderResources resources = msl.get_shader_resources();

                //                    	for (auto& resource : resources.sampled_images){
                //                   		unsigned set = msl.get_decoration(resource.id, spv::DecorationDescriptorSet);
                //                   		unsigned binding = msl.get_decoration(resource.id, spv::DecorationBinding);
                //                   		std::cout << "Image" << resource.name << "at set = " << set << ", binding = " << binding << std::endl;
                //                    	}

                //                    	spirv_cross::CompilerMSL::Options options;
                //                    	// options.msl_version = 50; // TODO: Add to list of options in .json
                //                    	msl.set_msl_options(options);
                //                    	std::string source = msl.compile();
                //                     std::cout << source << std::endl;
                //                     out_file << source;
                //                 }
                //                 else {
                //                     std::cout << "WARN: A file at the directory below cannot be created. Are you sure you have permission to write to this directory?\n" << shader_info[stage].write_dir << "\n" << shader_info[stage].write_dir + "/" + shader_info[stage].filenames[i] + ".spv";
                //                 }
                //             }
                //         }
                //     }
                }
            // } else {
            //     // SPIRV -> write_format
            // }
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
