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
	int map_index : MAP_INDEX;
	int tileset_index : TILESET_INDEX;
};

cbuffer ConstantBuffer : register(b0)
{
	row_major float4x4 mvp;
	int index;
	int tileset_index;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.map_index = index;
	output.tileset_index = tileset_index;
	output.pos = mul(float4(input.pos, 1), mvp);
	output.color = input.color;
	output.texcoord = input.texcoord;
	return output;
}