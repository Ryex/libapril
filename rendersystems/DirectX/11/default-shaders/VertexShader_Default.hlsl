// version 3.2

#include "Shaders.hlsli"

cbuffer constantBuffer : register(b0)
{
	min16float4x4 cMatrix;
	min16float4 cLerpAlpha;
};

struct VertexShaderInput
{
	min16float3 position : POSITION;
	min16float4 color : COLOR;
	float2 tex : TEXCOORD0;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput vertexShaderOutput;
	vertexShaderOutput.position = mul(min16float4(input.position, (min16float)1.0), cMatrix);
	vertexShaderOutput.color = input.color;
	vertexShaderOutput.tex = input.tex;
	vertexShaderOutput.lerpAlpha = cLerpAlpha;
	return vertexShaderOutput;
}
