#include "RenderTexture.h"
#include <d3d11.h>
#include "D3D.h"

RenderTexture::RenderTexture() = default;

RenderTexture::~RenderTexture()
{
	if (_pShaderResourceView)
	{
		_pShaderResourceView->Release();
		_pShaderResourceView = nullptr;
	}

	if (_pRenderTargetView)
	{
		_pRenderTargetView->Release();
		_pRenderTargetView = nullptr;
	}

	if (_pRenderTargetTexture)
	{
		_pRenderTargetTexture->Release();
		_pRenderTargetTexture = nullptr;
	}
}

bool RenderTexture::Initialise(ID3D11Device* device, const int width, const int height, const int mipMaps)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

	_width = width;
	_height = height;

	// Initialize the render target texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	// Setup the render target texture description.
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = mipMaps;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	// Create the render target texture.
	HRESULT result = device->CreateTexture2D(&textureDesc, nullptr, &_pRenderTargetTexture);
	if (FAILED(result))
	{
		return false;
	}

	// Setup the description of the render target view.
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	result = device->CreateRenderTargetView(_pRenderTargetTexture, &renderTargetViewDesc, &_pRenderTargetView);
	if (FAILED(result))
	{
		return false;
	}

	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	result = device->CreateShaderResourceView(_pRenderTargetTexture, &shaderResourceViewDesc, &_pShaderResourceView);
	return !FAILED(result);
}

ID3D11Texture2D* RenderTexture::GetTexture() const
{
	return _pRenderTargetTexture;
}

void RenderTexture::SetRenderTarget(D3D* d3d, ID3D11DeviceContext* deviceContext) const
{
	if (!d3d->ResizeDepthBuffer(_width, _height))
	{
		return;
	}

	d3d->ResizeViewport(_width, _height);

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	deviceContext->OMSetRenderTargets(1, &_pRenderTargetView, d3d->GetDepthStencilView());
}

void RenderTexture::ClearRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView,
                                      const float red, const float green, const float blue, const float alpha) const
{
	float color[4];

	// Setup the color to clear the buffer to.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// Clear the back buffer.
	deviceContext->ClearRenderTargetView(_pRenderTargetView, color);

	// Clear the depth buffer.
	deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

ID3D11ShaderResourceView* RenderTexture::GetSRV() const
{
	return _pShaderResourceView;
}
