struct vertex_input {
	float2 pos : POSITION;
	float2 uv : TEXCOORD0;
};

struct vertex_output {
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	float opacity : COLOR0;
};

cbuffer transforms : register(c0) {
	float4x4 world;
};


vertex_output main(vertex_input i) 
{
	vertex_output o;
	o.uv = i.uv;
	o.pos = float4(mul((float3x3)world,float3(i.pos, 1)).xy, .5, 1);
	o.opacity = world[0][3];
	return o;
}