#include "Texture.h"
#include "DDSTextureLoader.h"
#include <sstream>

using namespace DirectX;

Texture::Texture() = default;

Texture::~Texture()
{
	if (_pTexture)
	{
		_pTexture->Release();
		_pTexture = nullptr;
	}

	if (_pTextureSrv)
	{
		_pTextureSrv->Release();
		_pTextureSrv = nullptr;
	}
}

bool Texture::Initialise(ID3D11Device* device, const wchar_t* fileName)
{
	const std::wstring relPath = std::wstring(fileName);
	std::wstringstream str;

	// Since we're running DirectX, we don't have to worry about the lack of cross-platform for this API:
	const HMODULE module = GetModuleHandle(nullptr);
	if (module != nullptr)
	{
		WCHAR exePath[MAX_PATH];
		GetModuleFileName(module, exePath, (sizeof(exePath)));
		const std::wstring::size_type pos = std::wstring(exePath).find_last_of(L"\\/");
		str << std::wstring(exePath).substr(0, pos);
		str << "\\";
		str << relPath;
	}
	else
	{
		return false;
	}

	// Load the texture in.
	ID3D11Resource* texture;
	const HRESULT result = CreateDDSTextureFromFile(device, str.str().c_str(), &texture, &_pTextureSrv);
	if (FAILED(result))
	{
		return false;
	}

	_pTexture = static_cast<ID3D11Texture2D*>(texture);

	return true;
}

ID3D11Texture2D* Texture::GetTexture() const
{
	return _pTexture;
}

ID3D11ShaderResourceView* Texture::GetSRV() const
{
	return _pTextureSrv;
}
