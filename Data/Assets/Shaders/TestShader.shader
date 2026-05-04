#type layout

type Float3 POSITION
type Float2 TEXCOORD

#type vertex

cbuffer MatrixBuffer : register(b0)
{
	matrix u_ViewProjection;
};

cbuffer MatrixBuffer : register(b1)
{
	matrix u_Transform;
};

cbuffer VectorBuffer : register(b2)
{
	float3 u_Color;
	float padding;
};

struct VS_INPUT
{
	float3 Pos : POSITION;
	float2 TexCoords : TEXCOORD;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float4 Col : COLOR;
};

PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output;
	output.Pos = mul(float4(input.Pos, 1.f), u_Transform); // Pass position to rasterizer
	output.Pos = mul(output.Pos, u_ViewProjection); // Pass position to rasterizer
	output.Col = float4(u_Color.x, u_Color.y, u_Color.z, 1.0f); // Pass color to pixel shader
	return output;
}

#type fragment

Texture2D u_Texture : register(t0);
SamplerState u_Sampler : register(s0);

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float4 Col : COLOR;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	return input.Col;
}