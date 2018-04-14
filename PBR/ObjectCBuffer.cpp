#include "ObjectCBuffer.h"
#include <d3d11.h>

ObjectCBuffer::ObjectCBuffer()
{
}

ObjectCBuffer::~ObjectCBuffer()
{
}

bool ObjectCBuffer::Initialise(ID3D11Device* device)
{
	return CBuffer::Initialise(device, sizeof(ObjectBufferType));
}

bool ObjectCBuffer::Update(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, const XMFLOAT3 modelPos) const
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Transpose the matrices to prepare them for the shader.
	worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixTranslation(modelPos.x, modelPos.y, modelPos.z));
	worldMatrix = XMMatrixTranspose(worldMatrix);

	// Lock the constant buffer so it can be written to.
	const HRESULT result = deviceContext->Map(_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	ObjectBufferType* matrixPtr = static_cast<ObjectBufferType*>(mappedResource.pData);
	matrixPtr->World = worldMatrix;

	deviceContext->Unmap(_pBuffer, 0);

	return true;
}
