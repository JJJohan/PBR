#pragma once

#include "Shader.h"

struct HWND__;
struct ID3D11SamplerState;
class CBuffer;

class IntegrateBRDFShader : public Shader
{
public:
	IntegrateBRDFShader();
	virtual ~IntegrateBRDFShader();

	bool Initialise(ID3D11Device* device, HWND__* hwnd) override;
	bool Render(ID3D11DeviceContext* deviceContext, int indexCount, CBuffer* frameBuffer) const;
};
