#pragma once

struct MeshData;

class Shapes
{
	Shapes();
	~Shapes();

public:
	static void CreateSphere(MeshData& meshData, float radius, int sliceCount, int stackCount, int& vertexCount,
	                         int& indexCount);
	static void CreateCube(MeshData& meshData, int& vertexCount, int& indexCount);
};
