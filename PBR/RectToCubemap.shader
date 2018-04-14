cbuffer FrameBuffer
{
    matrix viewMatrix;
    matrix projectionMatrix;
	float4 lightPositions[4];
	float4 lightColours[4];
	float4 camPos;
	float4 customData;
};

struct VertexInputType
{
    float4 position: POSITION;
    float2 uv: TEXCOORD0;
};

struct PixelInputType
{
    float4 position: SV_POSITION;
	float3 localPos: TEXCOORD1;
};

Texture2D shaderTexture;
SamplerState textureSampler;

PixelInputType VSMain(VertexInputType input)
{
	PixelInputType output;

    output.localPos = input.position.xyz;

	float4x4 newView = viewMatrix;
	newView[3][0] = 0.0;
	newView[3][1] = 0.0;
	newView[3][2] = 0.0;
	
    input.position.w = 1.0f;
    output.position = mul(input.position, newView);
    output.position = mul(output.position, projectionMatrix);

    output.position = output.position.xyzw;
	output.position.z = output.position.w * 0.9999;

	return output;
}

static const float2 invAtan = float2(0.1591, 0.3183);
float2 SampleSphericalMap(float3 v)
{
    float2 uv = float2(atan2(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

float4 PSMain(PixelInputType input) : SV_TARGET
{
	float2 uv = SampleSphericalMap(normalize(input.localPos));
    float3 colour = shaderTexture.Sample(textureSampler, uv).rgb;
		
    return float4(colour, 1.0);
}