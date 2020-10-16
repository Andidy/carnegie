struct VS_INPUT
{
	float3 pos : POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
	float3 normal : NORMAL;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
};

cbuffer ConstantBuffer : register(b0)
{
	row_major float4x4 mvp;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.pos = mul(float4(input.pos, 1), mvp);
	output.color = input.color;
	output.texcoord = input.texcoord;
	return output;
}