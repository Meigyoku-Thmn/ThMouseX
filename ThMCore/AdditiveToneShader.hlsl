Texture2D<float4> Texture : register(t0);
sampler TextureSampler : register(s0);

float4 main(float4 color : COLOR0, float2 texCoord : TEXCOORD0) : SV_Target0
{
    float4 out_color = Texture.Sample(TextureSampler, texCoord);
    out_color.rgb = out_color.rgb + color.rgb;
    out_color.a = out_color.a * color.a;
    return out_color;
}