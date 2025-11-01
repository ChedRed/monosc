static const uint3 gl_WorkGroupSize = uint3(256u, 1u, 1u);

RWByteAddressBuffer _20 : register(u2);
ByteAddressBuffer _27 : register(t0);
ByteAddressBuffer _35 : register(t1);

static uint3 gl_GlobalInvocationID;
struct SPIRV_Cross_Input
{
    uint3 gl_GlobalInvocationID : SV_DispatchThreadID;
};

void comp_main()
{
    uint idx = gl_GlobalInvocationID.x;
    _20.Store(idx * 4 + 0, asuint(asfloat(_27.Load(idx * 4 + 0)) + asfloat(_35.Load(idx * 4 + 0))));
}

[numthreads(256, 1, 1)]
void main(SPIRV_Cross_Input stage_input)
{
    gl_GlobalInvocationID = stage_input.gl_GlobalInvocationID;
    comp_main();
}
