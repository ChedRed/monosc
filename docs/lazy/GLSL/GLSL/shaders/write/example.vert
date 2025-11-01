#version 450

layout(binding = 0, std140) uniform camera
{
    vec3 viewport;
} _16;

layout(location = 0) in vec2 posin;
layout(location = 0) out vec2 uvout;
layout(location = 1) in vec2 uvin;
layout(location = 1) out vec2 posout;

void main()
{
    vec2 n_pos = ((posin / _16.viewport.xy) * 2.0) - vec2(1.0);
    gl_Position = vec4(n_pos * vec2(_16.viewport.z, -_16.viewport.z), 0.0, 1.0);
    uvout = uvin;
    posout = posin;
}

