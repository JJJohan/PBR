#pragma once

#include "CBuffer.h"

struct ObjectBufferType
{
	XMMATRIX World;
};

class ObjectCBuffer : public CBuffer
{
public:
	ObjectCBuffer();
	virtual ~ObjectCBuffer();

	bool Initialise(ID3D11Device* device) override;
	bool Update(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMFLOAT3 modelPos) const;
};
