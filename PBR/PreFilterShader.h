#pragma once

#include "Shader.h"

struct ID3D11SamplerState;
class CBuffer;
struct HWND__;

class PreFilterShader : public Shader
{
public:
	PreFilterShader();
	virtual ~PreFilterShader();

	bool Initialise(ID3D11Device* device, HWND__* hwnd) override;
	bool Render(ID3D11DeviceContext* deviceContext, int indexCount, CBuffer* frameBuffer) const;

private:
	ID3D11SamplerState* _pSampler;
};
