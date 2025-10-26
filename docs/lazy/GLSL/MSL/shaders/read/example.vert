#version 460

layout(location = 0) in vec2 posin;
layout(location = 1) in vec2 uvin;

layout(location = 0) out vec2 uvout;
layout(location = 1) out vec2 posout;

layout(set = 1, binding = 0, std140) uniform camera { // Number of uniforms defined in SDL_CreateGPUShaderInfo
    vec3 viewport;
};

void main() {
    vec2 n_pos = ((posin / viewport.xy) * 2.) - 1;

    gl_Position = vec4(n_pos * vec2(viewport.z, -viewport.z), 0.0f, 1.0f);
    uvout = uvin;
    posout = posin;
}
