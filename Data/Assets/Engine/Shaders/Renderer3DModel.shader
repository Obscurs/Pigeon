#type layout

type Float3 POSITION
type Float3 NORMAL
type Float2 TEXCOORDS

#type vertex

cbuffer MatrixBuffer : register(b0)
{
	matrix u_ViewProjection;
};

struct VS_INPUT
{
	float3 a_Position : POSITION;
	float3 a_Normal : NORMAL;
	float2 a_TexCoord : TEXCOORDS;
};

struct PS_INPUT
{
	float4 Position : SV_POSITION;
};

PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output;
	// u_ViewProjection carries the full per-model viewProjection * world matrix (uploaded per draw),
	// using the same multiply convention as the 2D shaders.
	output.Position = float4(input.a_Position, 1.f);
	output.Position = mul(output.Position, u_ViewProjection);
	return output;
}

#type fragment

struct PS_INPUT
{
	float4 Position : SV_POSITION;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	// Flat unlit colour; depth testing alone gives the rotating model its solid 3D silhouette.
	return float4(0.85f, 0.55f, 0.2f, 1.f);
}
