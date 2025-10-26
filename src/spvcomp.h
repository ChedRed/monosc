#pragma once

#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>
#include <spirv_hlsl.hpp>
#include <spirv_msl.hpp>
#include <cstdint>

std::string compilespv(const std::vector<uint32_t> & spv_shadercode, std::string lang);
