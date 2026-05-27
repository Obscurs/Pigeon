#type layout

type Float3 POSITION
type Float4 COLOR
type Float2 TEXCOORDS

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
};

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
	float2 TexCoord : TEXCOORDS;
};

PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output;
	output.TexCoord = input.a_TexCoord;
	output.Position = float4(input.a_Position, 1.f);
	output.Color = input.a_Color;
	return output;
}

#type fragment

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
	float2 TexCoord : TEXCOORDS;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	//if (input.TexId == 0)
	//{
	//	return u_Texture.Sample(u_Sampler, input.TexCoord) * input.Color;
	//}
	//else
	//{
	//	return u_Texture2.Sample(u_Sampler, input.TexCoord) * input.Color;
	//}
	//return float4(1.0f, 0.0f, 0.0f, 1.0f);
	return input.Color;
}