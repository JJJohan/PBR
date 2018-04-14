#pragma once

struct HWND__;
struct ID3D11Device;
struct ID3D10Blob;
struct ID3D11DeviceContext;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
struct D3D11_INPUT_ELEMENT_DESC;

class Shader
{
public:
	virtual bool Initialise(ID3D11Device* device, HWND__* window) = 0;

protected:
	Shader();
	~Shader();

	void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND__* hwnd, const wchar_t* shaderFilename) const;
	void RenderShader(ID3D11DeviceContext*, int) const;
	bool LoadShader(ID3D11Device* device, HWND__* hwnd, const wchar_t* shaderFileName,
	                D3D11_INPUT_ELEMENT_DESC* inputLayout, int inputCount);

	ID3D11VertexShader* _pVertexShader;
	ID3D11PixelShader* _pPixelShader;
	ID3D11InputLayout* _pLayout;
};
