#pragma once

#include "Shader.h"

struct ID3D11SamplerState;
class CBuffer;
struct HWND__;

class PBRShader : public Shader
{
public:
	PBRShader();
	virtual ~PBRShader();

	bool Initialise(ID3D11Device* device, HWND__* hwnd) override;
	bool Render(ID3D11DeviceContext* deviceContext, int indexCount, CBuffer* frameBuffer, CBuffer* objectBuffer) const;

private:
	ID3D11SamplerState* _pSampler;
};
