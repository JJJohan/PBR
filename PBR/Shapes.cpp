#include "Shapes.h"
#include <cmath>
#include "Model.h"
#include "Graphics.h"

using namespace DirectX;

Shapes::Shapes()
{
}


Shapes::~Shapes()
{
}

void Shapes::CreateCube(MeshData& meshData, int& vertexCount, int& indexCount)
{
	vertexCount = 24;
	indexCount = 36;

	const float length = 1.0f;
	const float width = 1.0f;
	const float height = 1.0f;

	XMFLOAT3 p0 = XMFLOAT3(-length * .5f, -width * .5f, height * .5f);
	XMFLOAT3 p1 = XMFLOAT3(length * .5f, -width * .5f, height * .5f);
	XMFLOAT3 p2 = XMFLOAT3(length * .5f, -width * .5f, -height * .5f);
	XMFLOAT3 p3 = XMFLOAT3(-length * .5f, -width * .5f, -height * .5f);

	XMFLOAT3 p4 = XMFLOAT3(-length * .5f, width * .5f, height * .5f);
	XMFLOAT3 p5 = XMFLOAT3(length * .5f, width * .5f, height * .5f);
	XMFLOAT3 p6 = XMFLOAT3(length * .5f, width * .5f, -height * .5f);
	XMFLOAT3 p7 = XMFLOAT3(-length * .5f, width * .5f, -height * .5f);

	PosUvVertexType* vertices = new PosUvVertexType[24];

	vertices[0].Position = p0;
	vertices[1].Position = p1;
	vertices[2].Position = p2;
	vertices[3].Position = p3;

	vertices[4].Position = p7;
	vertices[5].Position = p4;
	vertices[6].Position = p0;
	vertices[7].Position = p3;

	vertices[8].Position = p4;
	vertices[9].Position = p5;
	vertices[10].Position = p1;
	vertices[11].Position = p0;

	vertices[12].Position = p6;
	vertices[13].Position = p7;
	vertices[14].Position = p3;
	vertices[15].Position = p2;

	vertices[16].Position = p5;
	vertices[17].Position = p6;
	vertices[18].Position = p2;
	vertices[19].Position = p1;

	vertices[20].Position = p7;
	vertices[21].Position = p6;
	vertices[22].Position = p5;
	vertices[23].Position = p4;

	XMFLOAT2 _00 = XMFLOAT2(0.0f, 0.0f);
	XMFLOAT2 _10 = XMFLOAT2(1.0f, 0.0f);
	XMFLOAT2 _01 = XMFLOAT2(0.0f, 1.0f);
	XMFLOAT2 _11 = XMFLOAT2(1.0f, 1.0f);

	for (int i = 0; i < 6; ++i)
	{
		vertices[i * 4].Uv = _11;
		vertices[i * 4 + 1].Uv = _01;
		vertices[i * 4 + 2].Uv = _00;
		vertices[i * 4 + 3].Uv = _10;
	}

	meshData.PosUvVertexData = vertices;

	meshData.IndexData = new unsigned long[36];
	int index = 0;
	for (int i = 0; i < 6; ++i)
	{
		meshData.IndexData[index++] = i * 4;
		meshData.IndexData[index++] = i * 4 + 1;
		meshData.IndexData[index++] = i * 4 + 3;

		meshData.IndexData[index++] = i * 4 + 1;
		meshData.IndexData[index++] = i * 4 + 2;
		meshData.IndexData[index++] = i * 4 + 3;
	}
}

void Shapes::CreateSphere(MeshData& meshData, float radius, int sliceCount, int stackCount, int& vertexCount,
                          int& indexCount)
{
	FullVertexType* vertices = new FullVertexType[sliceCount * stackCount + 1];
	meshData.IndexData = new unsigned long[sliceCount * 6 + (stackCount - 2) * sliceCount * 6];

	vertices[0].Position = XMFLOAT3(0.0f, radius, 0.0f);
	vertices[0].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertices[0].Uv = XMFLOAT2(0.0f, 0.0f);

	const float phiStep = XM_PI / stackCount;
	const float thetaStep = 2.0f * XM_PI / sliceCount;

	int vertexIndex = 1;
	for (int i = 1; i <= stackCount - 1; i++)
	{
		const float phi = i * phiStep;
		for (int j = 0; j <= sliceCount; j++)
		{
			const float theta = j * thetaStep;
			vertices[vertexIndex].Position = XMFLOAT3(
				radius * std::sin(phi) * std::cos(theta),
				radius * std::cos(phi),
				radius * std::sin(phi) * std::sin(theta));

			XMStoreFloat3(&vertices[vertexIndex].Normal,
			              XMVector3Normalize(XMLoadFloat3(&vertices[vertexIndex].Position)));

			vertices[vertexIndex].Uv = XMFLOAT2(theta / (XM_PI * 2), phi / XM_PI);

			++vertexIndex;
		}
	}

	vertices[vertexIndex].Position = XMFLOAT3(0.0f, -radius, 0.0f);
	vertices[vertexIndex].Normal = XMFLOAT3(0.0f, -1.0f, 0.0f);
	vertices[vertexIndex].Uv = XMFLOAT2(0.0f, 1.0f);

	++vertexIndex;
	meshData.FullVertexData = vertices;
	vertexCount = vertexIndex;

	int index = 0;
	for (int i = 1; i <= sliceCount; i++)
	{
		meshData.IndexData[index++] = 0;
		meshData.IndexData[index++] = i + 1;
		meshData.IndexData[index++] = i;
	}
	int baseIndex = 1;
	const int ringVertexCount = sliceCount + 1;
	for (int i = 0; i < stackCount - 2; i++)
	{
		for (int j = 0; j < sliceCount; j++)
		{
			meshData.IndexData[index++] = baseIndex + i * ringVertexCount + j;
			meshData.IndexData[index++] = baseIndex + i * ringVertexCount + j + 1;
			meshData.IndexData[index++] = baseIndex + (i + 1) * ringVertexCount + j;

			meshData.IndexData[index++] = baseIndex + (i + 1) * ringVertexCount + j;
			meshData.IndexData[index++] = baseIndex + i * ringVertexCount + j + 1;
			meshData.IndexData[index++] = baseIndex + (i + 1) * ringVertexCount + j + 1;
		}
	}
	const int southPoleIndex = vertexIndex - 1;
	baseIndex = southPoleIndex - ringVertexCount;
	for (int i = 0; i < sliceCount; i++)
	{
		meshData.IndexData[index++] = southPoleIndex;
		meshData.IndexData[index++] = baseIndex + i;
		meshData.IndexData[index++] = baseIndex + i + 1;
	}

	indexCount = index;
}
