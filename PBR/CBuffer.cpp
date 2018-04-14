#include "CBuffer.h"
#include <d3d11.h>

CBuffer::CBuffer() = default;

CBuffer::~CBuffer()
{
	if (_pBuffer)
	{
		_pBuffer->Release();
		_pBuffer = nullptr;
	}
}

bool CBuffer::Initialise(ID3D11Device* device, const size_t bufferSize)
{
	struct D3D11_BUFFER_DESC bufferDesc;

	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = UINT(bufferSize);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	const HRESULT result = device->CreateBuffer(&bufferDesc, nullptr, &_pBuffer);
	return !FAILED(result);
}

ID3D11Buffer* CBuffer::GetBuffer() const
{
	return _pBuffer;
}
