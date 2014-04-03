// version 3.2

#include "Shaders.hlsli"

min16float4 main(PixelShaderInput input) : SV_Target
{
	return min16float4(lerp(min16float3((min16float)1.0, (min16float)1.0, (min16float)1.0), input.color.rgb, input.color.a), input.lerpAlpha.a);
}
