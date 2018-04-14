#include "FrameCBuffer.h"
#include <d3d11.h>

FrameCBuffer::FrameCBuffer()
{
}

FrameCBuffer::~FrameCBuffer()
{
}

bool FrameCBuffer::Initialise(ID3D11Device* device)
{
	return CBuffer::Initialise(device, sizeof(FrameBufferType));
}

bool FrameCBuffer::Update(ID3D11DeviceContext* deviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
                          const XMFLOAT3 camPos) const
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Transpose the matrices to prepare them for the shader.
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	// Lock the constant buffer so it can be written to.
	const HRESULT result = deviceContext->Map(_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	FrameBufferType* matrixPtr = static_cast<FrameBufferType*>(mappedResource.pData);
	matrixPtr->View = viewMatrix;
	matrixPtr->Projection = projectionMatrix;
	matrixPtr->CamPos = XMFLOAT4(camPos.x, camPos.y, camPos.z, 0.0f);
	matrixPtr->CustomData = _custom;

	int lightIndex = 0;
	for (int i = 0; i < 2; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			matrixPtr->LightPositions[lightIndex] = XMFLOAT4(2.5f + i * 10.0f, 2.5f + j * 10.0f, -10.0f, 0.0f);
			matrixPtr->LightColours[lightIndex] = XMFLOAT4(300.0f, 300.0f, 300.0f, 0.0f);
			++lightIndex;
		}
	}

	deviceContext->Unmap(_pBuffer, 0);

	return true;
}

void FrameCBuffer::SetCustomFloat(const int index, const float value)
{
	if (index == 0) _custom.x = value;
	else if (index == 1) _custom.y = value;
	else if (index == 2) _custom.z = value;
	else _custom.w = value;
}
