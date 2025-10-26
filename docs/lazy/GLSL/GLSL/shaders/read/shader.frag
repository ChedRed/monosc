#version 460

layout(location = 0) in vec2 uv;
layout(location = 1) in vec2 pos;
layout(location = 0) out vec4 color;

layout(set = 3, binding = 0, std140) uniform camera { // Number of uniforms defined in SDL_CreateGPUShaderInfo
    vec4 viewport;
};

void main()
{
    vec2 n_pos = vec2(int(pos.x), int(pos.y));
    vec3 n_color = vec3(1, 1, 1);
    if (n_pos.y - viewport.y > 8 && viewport.w - (n_pos.y - viewport.y) > 8) {
        n_color = vec3(0, 0, 0);
    }
    // int t_val = int(pos.x + pos.y); // float(t_val % 2)
    // vec3 t_color = vec3(pos, 0);
    color = vec4(n_color, 1);
}
