#pragma once

#include <DirectXMath.h>

using namespace DirectX;

struct ID3D11Device;
struct ID3D11Buffer;
struct ID3D11DeviceContext;

class CBuffer
{
public:
	virtual bool Initialise(ID3D11Device* device) = 0;
	ID3D11Buffer* GetBuffer() const;

protected:
	CBuffer();
	~CBuffer();
	bool Initialise(ID3D11Device* device, size_t bufferSize);

	ID3D11Buffer* _pBuffer;
};
