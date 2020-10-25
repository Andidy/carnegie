Texture2D tilemap : register(t0);
Texture2D tileset : register(t1);
SamplerState pix : register(s0);
SamplerState lin : register(s1);

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
	const int2 tilemap_dim = { 10, 10 };
	const float2 f_tilemap_dim = { 10, 10 };

	const int2 tileset_dim = { 16, 16 };
	const float2 f_tileset_dim = { 16, 16 };
	
	const float E = 0.00001;
	float2 f_tilemap_coord = min(input.texcoord * f_tilemap_dim, f_tilemap_dim - E);
	float2 tilemap_coord = float2(int2(f_tilemap_coord));
	float2 tileset_uv = frac(f_tilemap_coord);

	tilemap_coord.x = tilemap_coord.x + 0.5;
	tilemap_coord.y = tilemap_coord.y + 0.5;
	float tile_index = 255.0 * tilemap.Sample(pix, tilemap_coord / f_tilemap_dim).r;

	float2 aauv = tileset_uv * float2(16, 16);
	float2 alpha = 0.7 * float2(ddx(aauv.x), ddy(aauv.y));
	float2 x = frac(aauv);

	float2 x_ = clamp(0.5 / alpha * x, 0.0, 0.5) + clamp(0.5 / alpha * (x - 1.0) + 0.5, 0.0, 0.5);

	float2 texCoord = (floor(aauv) + x_) / float2(16, 16);

	float3 col = tileset.Sample(lin, float3(texCoord, tile_index)).rgb;

	return float4(col, 1.0);
	// return t1.Sample(s1, input.texcoord) * input.color;
}