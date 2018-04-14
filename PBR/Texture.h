#pragma once

struct ID3D11Device;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

class Texture
{
public:
	Texture();
	~Texture();

	bool Initialise(ID3D11Device* device, const wchar_t* fileName);

	ID3D11Texture2D* GetTexture() const;
	ID3D11ShaderResourceView* GetSRV() const;

private:
	ID3D11Texture2D* _pTexture;
	ID3D11ShaderResourceView* _pTextureSrv;
};
