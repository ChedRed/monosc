#include "spvcomp.h"
#include <iostream>

std::string compilespv(const std::vector<uint32_t> & spv_shadercode, std::string lang){
    if (lang == "glsl"){
        spirv_cross::CompilerGLSL glsl(std::move(spv_shadercode));
       	spirv_cross::ShaderResources resources = glsl.get_shader_resources();

       	for (auto &resource : resources.sampled_images){
      		unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
      		unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
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
        std::cout << "thisthat" << std::endl;
        std::cout << spv_shadercode.data() << std::endl;
        spirv_cross::CompilerHLSL hlsl(std::move(spv_shadercode));
        std::cout << "then" << std::endl;
       	spirv_cross::ShaderResources resources = hlsl.get_shader_resources();

       	for (auto &resource : resources.sampled_images){
      		unsigned set = hlsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
      		unsigned binding = hlsl.get_decoration(resource.id, spv::DecorationBinding);
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
      		unsigned set = msl.get_decoration(resource.id, spv::DecorationDescriptorSet);
      		unsigned binding = msl.get_decoration(resource.id, spv::DecorationBinding);
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
