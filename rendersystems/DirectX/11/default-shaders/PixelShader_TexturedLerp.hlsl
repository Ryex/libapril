// version 3.2

#include "Shaders.hlsli"

Texture2D<min16float4> cTexture : register(t0);
SamplerState cSampler : register(s0);

min16float4 main(PixelShaderInput input) : SV_Target
{
	min16float4 tex = cTexture.Sample(cSampler, input.tex);
	return min16float4(lerp(tex.rgb, input.color.rgb, input.color.a), tex.a * input.lerpAlpha.a);
}
