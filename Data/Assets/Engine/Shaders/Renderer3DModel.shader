#type layout

type Float3 POSITION
type Float3 NORMAL
type Float2 TEXCOORDS

#type vertex

cbuffer ViewProjectionBuffer : register(b0)
{
	matrix u_ViewProjection;
};

cbuffer TransformBuffer : register(b1)
{
	matrix u_Transform;
};

cbuffer LightBuffer : register(b2)
{
	float3 u_LightPos;
	float u_LightPad;
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
	float3 Color : COLOR0;
};

PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output;

	// u_ViewProjection carries the full per-model viewProjection * world matrix (clip space), while
	// u_Transform is the model's world matrix on its own (for lighting). Same multiply convention as
	// the 2D shaders.
	output.Position = mul(float4(input.a_Position, 1.f), u_ViewProjection);

	// Basic Lambert diffuse, with the light at the camera position. Per-vertex (Gouraud) because the
	// constant buffers are only bound to the vertex stage.
	float3 worldPosition = mul(float4(input.a_Position, 1.f), u_Transform).xyz;
	float3 worldNormal = normalize(mul(input.a_Normal, (float3x3)u_Transform));
	float3 lightDirection = normalize(u_LightPos - worldPosition);
	float diffuse = max(dot(worldNormal, lightDirection), 0.f);

	float3 baseColor = float3(0.85f, 0.55f, 0.2f);
	float ambient = 0.2f;
	output.Color = baseColor * (ambient + (1.f - ambient) * diffuse);
	return output;
}

#type fragment

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float3 Color : COLOR0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	return float4(input.Color, 1.f);
}
