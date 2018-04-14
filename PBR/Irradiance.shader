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

static const float PI = 3.14159265359;

float4 PSMain(PixelInputType input) : SV_TARGET
{
	float3 normal = input.localPos;
	float3 irradiance = float3(0.0, 0.0, 0.0);

	float3 up = float3(0.0, 1.0, 0.0);
	float3 right = cross(up, normal);
	up = cross(normal, right);

	float sampleDelta = 0.025;
	float nrSamples = 0.0;
	for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			// spherical to cartesian (in tangent space)
			float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			// tangent space to world
			float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

			irradiance += shaderTexture.Sample(textureSampler, sampleVec).rgb * cos(theta) * sin(theta);
			nrSamples++;
		}
	}
	irradiance = PI * irradiance * (1.0 / float(nrSamples));

	return float4(irradiance, 1.0);
}