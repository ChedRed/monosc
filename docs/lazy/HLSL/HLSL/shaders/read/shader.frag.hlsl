cbuffer camera : register(b0)
{
    float4 _38_viewport : packoffset(c0);
};


static float2 pos;
static float4 color;
static float2 uv;

struct SPIRV_Cross_Input
{
    float2 uv : TEXCOORD0;
    float2 pos : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float4 color : SV_Target0;
};

void frag_main()
{
    float2 n_pos = float2(float(int(pos.x)), float(int(pos.y)));
    float3 n_color = 1.0f.xxx;
    bool _45 = (n_pos.y - _38_viewport.y) > 8.0f;
    bool _58;
    if (_45)
    {
        _58 = (_38_viewport.w - (n_pos.y - _38_viewport.y)) > 8.0f;
    }
    else
    {
        _58 = _45;
    }
    if (_58)
    {
        n_color = 0.0f.xxx;
    }
    color = float4(n_color, 1.0f);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    pos = stage_input.pos;
    uv = stage_input.uv;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.color = color;
    return stage_output;
}
