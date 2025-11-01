#version 450

layout(binding = 0, std140) uniform camera
{
    vec4 viewport;
} _38;

layout(location = 1) in vec2 pos;
layout(location = 0) out vec4 color;
layout(location = 0) in vec2 uv;

void main()
{
    vec2 n_pos = vec2(float(int(pos.x)), float(int(pos.y)));
    vec3 n_color = vec3(1.0);
    bool _45 = (n_pos.y - _38.viewport.y) > 8.0;
    bool _58;
    if (_45)
    {
        _58 = (_38.viewport.w - (n_pos.y - _38.viewport.y)) > 8.0;
    }
    else
    {
        _58 = _45;
    }
    if (_58)
    {
        n_color = vec3(0.0);
    }
    color = vec4(n_color, 1.0);
}

