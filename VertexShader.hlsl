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
	int spritesheet_offset : SPRITESHEET_OFFSET;
	int anim_frame : ANIM_FRAME;
};

cbuffer ConstantBuffer : register(b0)
{
	row_major float4x4 mvp;
	int is_animated;
	int layer_index;
	int spritesheet_offset;
	int anim_frame;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.pos = mul(float4(input.pos, 1), mvp);
	output.color = input.color;
	output.texcoord = input.texcoord;
	
	output.is_animated = is_animated;
	output.layer_index = layer_index;
	output.spritesheet_offset = spritesheet_offset;
	output.anim_frame = anim_frame;
	return output;
}