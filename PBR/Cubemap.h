#pragma once

#include <vector>

class RenderTexture;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

class Cubemap
{
public:
	Cubemap();
	~Cubemap();

	bool Initialise(ID3D11Device* device, ID3D11DeviceContext* context, std::vector<RenderTexture*> faces, int width,
	                int height, int mipMaps);
	void Copy(ID3D11DeviceContext* context, std::vector<RenderTexture*> faces, int width, int height, int mipSlice) const;

	ID3D11Texture2D* GetTexture() const;
	ID3D11ShaderResourceView* GetSRV() const;

private:
	ID3D11Texture2D* _pTexture;
	ID3D11ShaderResourceView* _pShaderResourceView;
	int _mipMaps;
};
