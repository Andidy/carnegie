Texture2D tex[] : register(t0);
SamplerState pix : register(s0);
SamplerState lin : register(s1);

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



float4 main(VS_OUTPUT input) : SV_TARGET
{
	const int2 tile_dim = {32, 32};
	
	const int2 tilemap_dim = { 4320, 2160 };
	const float2 f_tilemap_dim = { 4320, 2160 };

	const int2 tileset_dim = { 16, 16 };
	const float2 f_tileset_dim = { 16, 16 };
	
	const float E = 0.00001;
	float2 f_tilemap_coord = min(input.texcoord * f_tilemap_dim, f_tilemap_dim - E);
	float2 tilemap_coord = float2(int2(f_tilemap_coord));
	float2 tileset_uv = frac(f_tilemap_coord);

	tilemap_coord.x = tilemap_coord.x + 0.5;
	tilemap_coord.y = tilemap_coord.y + 0.5;
	int4 tile_data = int4(255.0 * tex[input.layer_index].Sample(pix, tilemap_coord / f_tilemap_dim).rgba);

	int unit_spritesheet_index = tile_data.r + input.spritesheet_offset;
	int anim_base_index = tile_data.g;

	int2 index_into_spritesheet; // = int2(input.anim_frame, anim_base_index);

	if (input.is_animated == 1) 
	{
		index_into_spritesheet = int2(input.anim_frame, anim_base_index);
	}
	else 
	{
		index_into_spritesheet = int2(anim_base_index % tileset_dim.x, anim_base_index / tileset_dim.y);
	}

	// tileset_uv.x += 0.5;
	tileset_uv = (tileset_uv + float2(index_into_spritesheet)) / f_tileset_dim;

	float4 col = tex[unit_spritesheet_index].Sample(pix, tileset_uv).rgba;
	
	if (col.a < 0.9) discard;

	return float4(col.rgb, 1.0);

	//float2 aauv = tileset_uv * float2(tile_dim);
	//float2 alpha = 0.7 * float2(ddx(aauv.x), ddy(aauv.y));
	//float2 x = frac(aauv);

	//float2 x_ = clamp(0.5 / alpha * x, 0.0, 0.5) + clamp(0.5 / alpha * (x - 1.0) + 0.5, 0.0, 0.5);

	//float2 texCoord = (floor(aauv) + x_) / float2(tile_dim);

	//float3 col = tileset.Sample(lin, float3(tileset_uv, tile_index)).rgb;

	//return float4(col, 1.0);
	// return t1.Sample(s1, input.texcoord) * input.color;
}