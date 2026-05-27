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
SamplerState u_Sampler : register(s0);

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
	float2 TexCoord : TEXCOORDS;
	int TexId : TEXID;
};

float screenPxRange(float2 texCoord) {
	const float pxRange = 2.0; // set to distance field's pixel range
	uint width, height;
    u_Texture.GetDimensions(width, height);
    float2 unitRange = float2(pxRange, pxRange)/float2(width, height);

	float dx = ddx(texCoord.x);
	float dy = ddy(texCoord.y);
	float fwidth = abs(dx) + abs(dy);
    float2 screenTexSize = float2(1.0, 1.0)/fwidth;
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

float4 main(PS_INPUT input) : SV_TARGET
{
	float3 msd = u_Texture.Sample(u_Sampler, input.TexCoord).rgb;
	float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = screenPxRange(input.TexCoord)*(sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
	if (opacity == 0.0)
		discard;

	float4 bgColor = float4(0.0, 0.0, 0.0, 0.0);
    float4 outColor = lerp(bgColor, input.Color, opacity);
	if (outColor.a == 0.0)
		discard;

	//float4 outColor = u_Texture.Sample(u_Sampler, input.TexCoord) * input.Color;
	return outColor;
}