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
	int is_animated : IS_ANIMATED;
	int layer_index : LAYER_INDEX;
	int tileset_base_index : TILESET_BASE_INDEX;
	int anim_frame : ANIM_FRAME;
	float anim_time : ANIM_TIME;
};

cbuffer ConstantBuffer : register(b0)
{
	row_major float4x4 mvp;
	int is_animated;
	int layer_index;
	int tileset_base_index;
	int anim_frame;
	float anim_time;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.pos = mul(float4(input.pos, 1), mvp);
	output.color = input.color;
	output.texcoord = input.texcoord;
	
	output.is_animated = is_animated;
	output.layer_index = layer_index;
	output.tileset_base_index = tileset_base_index;
	output.anim_frame = anim_frame;
	output.anim_time = anim_time;
	return output;
}