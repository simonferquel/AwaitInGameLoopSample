struct vertex_output {
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	float opacity : COLOR0;
};
texture2D tex;
SamplerState linear_sampler {
	MIN_MAG = LINEAR;
};


float4 main(vertex_output i) : SV_TARGET
{
	float4 color = tex.Sample(linear_sampler, i.uv);
	color.a *= i.opacity;
	return color;

}