#include "glslcomp.h"
#include <iostream>

glslangwrap::glslangwrap(){
    glslang::InitializeProcess();
}

bool glslangwrap::compileglsl(const char * source, std::vector<uint32_t> & spv_shadercode, glslang::EShSource read_source, EShLanguage stage){
    std::ifstream prefile(source);
    if (!prefile){
        // std::cout << "WARN: The file below was skipped. Are you sure you have permission to read this file?\n" << source; // TODO: Turn into warn
        exit(1);
    }
    std::string file_string((std::istreambuf_iterator<char>(prefile)), std::istreambuf_iterator<char>());
    const char * file_code = file_string.c_str();

    glslang::TShader shader(stage);
    shader.setStrings(&file_code, 1);
    shader.setEnvInput(read_source, stage, glslang::EShClientVulkan, 450);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2); // TODO: Make versions a config setting
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);
    TBuiltInResource resources = *GetDefaultResources();
    if (!shader.parse(&resources, 450, false, EShMsgDefault)) {
        // exitmsg(1, std::string("Error: shader file ") + source + " is invalid!\n" + shader.getInfoLog()); // TODO: Get exitsmsg back
        exit(1);
    }
    glslang::TProgram program;
    program.addShader(&shader);

    if (!program.link(EShMsgDefault)) {
        // exitmsg(1, std::string("Error: shader file ") + source + " is invalid!\n" + program.getInfoLog()); // TODO: Get exitsmsg back
        exit(1);
    }

    glslang::TIntermediate * intermediate = program.getIntermediate(stage);
    if (!intermediate) {
        // exitmsg(1, std::string("Error: intermediate failed!\n") + source + "\n" + shader.getInfoLog()); // TODO: Get exitsmsg back
        exit(1);
    }

    glslang::GlslangToSpv(*intermediate, spv_shadercode);

    return true;
}

void glslangwrap::destroy(){
    glslang::FinalizeProcess();
}
