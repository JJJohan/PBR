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
	float3 localPos: TEXCOORD0;
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

float RadicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
float2 Hammersley(uint i, uint N)
{
	return float2(float(i) / float(N), RadicalInverse_VdC(i));
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
	float a = roughness * roughness;

	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	// from spherical coordinates to cartesian coordinates
	float3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	// from tangent-space vector to world-space sample vector
	float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
	float3 tangent = normalize(cross(up, N));
	float3 bitangent = cross(N, tangent);

	float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

float4 PSMain(PixelInputType input) : SV_TARGET
{
	float roughness = customData.x;
	float3 N = normalize(input.localPos);
	float3 R = N;
	float3 V = R;

	const uint SAMPLE_COUNT = 1024u;
	float totalWeight = 0.0;
	float3 prefilteredColor = float3(0.0, 0.0, 0.0);
	for (uint i = 0u; i < SAMPLE_COUNT; ++i)
	{
		float2 Xi = Hammersley(i, SAMPLE_COUNT);
		float3 H = ImportanceSampleGGX(Xi, N, roughness);
		float3 L = normalize(2.0 * dot(V, H) * H - V);

		float NdotL = max(dot(N, L), 0.0);
		if (NdotL > 0.0)
		{
			prefilteredColor += shaderTexture.Sample(textureSampler, L).rgb * NdotL;
			totalWeight += NdotL;
		}
	}
	prefilteredColor = prefilteredColor / totalWeight;

	return float4(prefilteredColor, 1.0);
}