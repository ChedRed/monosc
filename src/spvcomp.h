#pragma once

#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>
#include <spirv_cross/spirv_hlsl.hpp>
#include <spirv_cross/spirv_msl.hpp>

std::string compilespv(const std::vector<uint32_t> & spv_shadercode, std::string lang);
