#include "Model.h"
#include "Shapes.h"
#include "Graphics.h"
#include <d3d11.h>

using namespace DirectX;

Model::Model() = default;

Model::~Model()
{
	if (_pIndexBuffer)
	{
		_pIndexBuffer->Release();
		_pIndexBuffer = nullptr;
	}

	if (_pVertexBuffer)
	{
		_pVertexBuffer->Release();
		_pVertexBuffer = nullptr;
	}
}

bool Model::Initialise(ID3D11Device* device, const XMFLOAT3 position)
{
	_position = position;
	return InitialiseBuffers(device);
}

void Model::Render(ID3D11DeviceContext* deviceContext) const
{
	RenderBuffers(deviceContext);
}

int Model::GetIndexCount() const
{
	return _indexCount;
}

bool Model::InitialiseBuffers(ID3D11Device* device)
{
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	MeshData meshData;

	Shapes::CreateSphere(meshData, 1.0f, 20, 20, _vertexCount, _indexCount);

	FullVertexType* vertices = meshData.FullVertexData;
	unsigned long* indices = meshData.IndexData;

	// Colour the model.
	for (int i = 0; i < _vertexCount; ++i)
	{
		vertices[i].Colour = XMFLOAT4(1.0f, 0.6172f, 0.1384f, 1.0f); // Gold
	}

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(FullVertexType) * _vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	HRESULT result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &_pVertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	delete[] vertices;

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * _indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &_pIndexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	delete[] indices;

	return true;
}

XMFLOAT3 Model::GetPosition() const
{
	return _position;
}

void Model::RenderBuffers(ID3D11DeviceContext* deviceContext) const
{
	unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(FullVertexType);
	offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &_pVertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
