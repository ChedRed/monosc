cbuffer camera : register(b0)
{
    float3 _16_viewport : packoffset(c0);
};


static float4 gl_Position;
static float2 posin;
static float2 uvout;
static float2 uvin;
static float2 posout;

struct SPIRV_Cross_Input
{
    float2 posin : TEXCOORD0;
    float2 uvin : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float2 uvout : TEXCOORD0;
    float2 posout : TEXCOORD1;
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    float2 n_pos = ((posin / _16_viewport.xy) * 2.0f) - 1.0f.xx;
    gl_Position = float4(n_pos * float2(_16_viewport.z, -_16_viewport.z), 0.0f, 1.0f);
    uvout = uvin;
    posout = posin;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    posin = stage_input.posin;
    uvin = stage_input.uvin;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.uvout = uvout;
    stage_output.posout = posout;
    return stage_output;
}
