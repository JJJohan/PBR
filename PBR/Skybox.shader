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

TextureCube shaderTexture;
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

float4 PSMain(PixelInputType input) : SV_TARGET
{
    float3 colour = shaderTexture.Sample(textureSampler, input.localPos).rgb;

    colour = colour / (colour + float3(1.0, 1.0, 1.0));
    colour = pow(colour, float3(1.0/2.2, 1.0/2.2, 1.0/2.2)); 

    return float4(colour, 1.0);
}