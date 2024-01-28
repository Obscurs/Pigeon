#type vertex

cbuffer MatrixBuffer : register(b0)
{
	matrix u_ViewProjection;
};

cbuffer MatrixBuffer : register(b1)
{
	matrix u_Transform;
};

struct VS_INPUT
{
	float3 a_Position : POSITION;
	float2 a_TexCoord : TEXCOORD;
};

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output;
	output.TexCoord = input.a_TexCoord;
	output.Position = mul(float4(input.a_Position, 1.f), u_Transform);
	output.Position = mul(output.Position, u_ViewProjection);
	return output;
}

#type fragment

Texture2D u_Texture : register(t0);
SamplerState u_Sampler : register(s0);

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	return u_Texture.Sample(u_Sampler, input.TexCoord);
}