#type layout

type Float3 POSITION
type Float4 COLOR
type Float2 TEXCOORDS
type Int TEXID

#type vertex

cbuffer MatrixBuffer : register(b0)
{
	matrix u_ViewProjection;
};

struct VS_INPUT
{
	float3 a_Position : POSITION;
	float4 a_Color : COLOR;
	float2 a_TexCoord : TEXCOORDS;
	int a_TexId : TEXID;
};

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
	float2 TexCoord : TEXCOORDS;
	int TexId : TEXID;
};

PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output;
	output.TexCoord = input.a_TexCoord;
	output.Position = float4(input.a_Position, 1.f);
	output.Position = mul(output.Position, u_ViewProjection);
	output.Color = input.a_Color;
	output.TexId = input.a_TexId;
	return output;
}

#type fragment

Texture2D u_Texture : register(t0);
Texture2D u_Texture2 : register(t1);
//Texture2DArray u_Textures : register(t0);
SamplerState u_Sampler : register(s0);

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
	float2 TexCoord : TEXCOORDS;
	int TexId : TEXID;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	if (input.TexId == 0.0f)
	{
		return u_Texture.Sample(u_Sampler, input.TexCoord) * input.Color;
	}
	else
	{
		return u_Texture2.Sample(u_Sampler, input.TexCoord) * input.Color;
	}
	//return u_Textures.Sample(u_Sampler, float3(input.TexCoord, 1)) * input.Color;
}