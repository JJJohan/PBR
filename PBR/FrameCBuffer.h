#pragma once

#include "CBuffer.h"

struct FrameBufferType
{
	XMMATRIX View;
	XMMATRIX Projection;
	XMFLOAT4 LightPositions[4];
	XMFLOAT4 LightColours[4];
	XMFLOAT4 CamPos;
	XMFLOAT4 CustomData;
};

class FrameCBuffer : public CBuffer
{
public:
	FrameCBuffer();
	virtual ~FrameCBuffer();

	bool Initialise(ID3D11Device* device) override;
	bool Update(ID3D11DeviceContext* deviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 camPos) const;
	void SetCustomFloat(int index, float value);

private:
	XMFLOAT4 _custom;
};
