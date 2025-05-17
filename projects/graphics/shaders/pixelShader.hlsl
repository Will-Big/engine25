struct VSOut
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 PSMain(VSOut input) : SV_TARGET
{
    return float4(input.uv, 0, 1); // UV를 색으로 출력
}