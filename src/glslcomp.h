#pragma once

#include <SPIRV/GlslangToSpv.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <cstdint>
#include <fstream>
#include <vector>

class glslangwrap {
public:
glslangwrap();
bool compileglsl(const char * source, std::vector<uint32_t> & spv_shadercode, glslang::EShSource read_source, EShLanguage stage);
void destroy();
};
