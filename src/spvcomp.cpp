#include "spvcomp.h"
#include <iostream>

std::string compilespv(const std::vector<uint32_t> & spv_shadercode, std::string lang){
    if (lang == "glsl"){
        spirv_cross::CompilerGLSL glsl(spv_shadercode.data(), spv_shadercode.size());
       	spirv_cross::ShaderResources resources = glsl.get_shader_resources();

       	for (auto &resource : resources.sampled_images){
      		glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
      		glsl.get_decoration(resource.id, spv::DecorationBinding);
      		// std::cout << "Image" << resource.name << "at set = " << set << ", binding = " << binding << std::endl;
       	}

       	spirv_cross::CompilerGLSL::Options options;
       	options.version = 450;
       	options.es = false; // TODO: Add to list of options in .json
       	glsl.set_common_options(options);
       	std::string source = glsl.compile();
        return source;
    }

    if (lang == "hlsl"){
        spirv_cross::CompilerHLSL hlsl(spv_shadercode.data(), spv_shadercode.size());
       	spirv_cross::ShaderResources resources = hlsl.get_shader_resources();

       	for (auto &resource : resources.sampled_images){
      		hlsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
      		hlsl.get_decoration(resource.id, spv::DecorationBinding);
      		// std::cout << "Image" << resource.name << "at set = " << set << ", binding = " << binding << std::endl;
       	}

       	spirv_cross::CompilerHLSL::Options options;
        options.shader_model = 50;
       	hlsl.set_hlsl_options(options);
       	std::string source = hlsl.compile();
        return source;
    }

    if (lang == "msl"){
        spirv_cross::CompilerMSL msl(spv_shadercode.data(), spv_shadercode.size());
       	spirv_cross::ShaderResources resources = msl.get_shader_resources();

       	for (auto &resource : resources.sampled_images){
      		msl.get_decoration(resource.id, spv::DecorationDescriptorSet);
      		msl.get_decoration(resource.id, spv::DecorationBinding);
      		// std::cout << "Image" << resource.name << "at set = " << set << ", binding = " << binding << std::endl;
       	}

       	spirv_cross::CompilerMSL::Options options;
        // options.set_msl_version(1); // TODO: Set MSL version, also add to thingy
       	msl.set_msl_options(options);
       	std::string source = msl.compile();
        return source;
    }
    exit(1); // TODO: Bring exitmsg back
}
