#pragma once

struct ID3D11ShaderResourceView;
struct ID3D11Device;
struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11DeviceContext;
struct ID3D11DepthStencilView;
class D3D;

class RenderTexture
{
public:
	RenderTexture();
	~RenderTexture();

	bool Initialise(ID3D11Device* device, int width, int height, int mipMaps);

	void SetRenderTarget(D3D* d3d, ID3D11DeviceContext* deviceContext) const;
	void ClearRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView, float red,
	                       float green, float blue, float alpha) const;
	ID3D11ShaderResourceView* GetSRV() const;
	ID3D11Texture2D* GetTexture() const;

private:
	int _width;
	int _height;
	ID3D11Texture2D* _pRenderTargetTexture;
	ID3D11RenderTargetView* _pRenderTargetView;
	ID3D11ShaderResourceView* _pShaderResourceView;
};
