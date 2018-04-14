#include "Cubemap.h"
#include "RenderTexture.h"
#include <d3d11.h>

Cubemap::Cubemap() = default;

Cubemap::~Cubemap()
{
	if (_pTexture)
	{
		_pTexture->Release();
		_pTexture = nullptr;
	}

	if (_pShaderResourceView)
	{
		_pShaderResourceView->Release();
		_pShaderResourceView = nullptr;
	}
}

bool Cubemap::Initialise(ID3D11Device* device, ID3D11DeviceContext* context, std::vector<RenderTexture*> faces,
                         const int width, const int height, const int mipMaps)
{
	_mipMaps = mipMaps;

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = mipMaps;
	texDesc.ArraySize = 6;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	HRESULT result = device->CreateTexture2D(&texDesc, nullptr, &_pTexture);
	if (FAILED(result))
	{
		return false;
	}

	if (!faces.empty())
	{
		Copy(context, faces, width, height, 0);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = texDesc.MipLevels;
	srvDesc.TextureCube.MostDetailedMip = 0;

	result = device->CreateShaderResourceView(_pTexture, &srvDesc, &_pShaderResourceView);
	return !FAILED(result);
}

void Cubemap::Copy(ID3D11DeviceContext* context, std::vector<RenderTexture*> faces, const int width, const int height,
                   const int mipSlice) const
{
	D3D11_BOX sourceRegion;
	for (int i = 0; i < 6; ++i)
	{
		RenderTexture* texture = faces[i];

		sourceRegion.left = 0;
		sourceRegion.right = width;
		sourceRegion.top = 0;
		sourceRegion.bottom = height;
		sourceRegion.front = 0;
		sourceRegion.back = 1;

		context->CopySubresourceRegion(_pTexture, D3D11CalcSubresource(mipSlice, i, _mipMaps), 0, 0, 0, texture->GetTexture(),
		                               0, &sourceRegion);
	}
}

ID3D11Texture2D* Cubemap::GetTexture() const
{
	return _pTexture;
}

ID3D11ShaderResourceView* Cubemap::GetSRV() const
{
	return _pShaderResourceView;
}
