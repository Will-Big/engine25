struct VSIn
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
};
struct VSOut
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

VSOut VSMain(VSIn input)
{
    VSOut o;
    o.pos = float4(input.pos, 1.0);
    o.uv = input.uv;
    return o;
}