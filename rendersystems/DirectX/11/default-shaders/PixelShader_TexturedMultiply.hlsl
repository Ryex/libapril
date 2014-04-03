// version 3.2

#include "Shaders.hlsli"

Texture2D<min16float4> cTexture : register(t0);
SamplerState cSampler : register(s0);

min16float4 main(PixelShaderInput input) : SV_Target
{
	return (cTexture.Sample(cSampler, input.tex) * input.color);
}
