#pragma once

struct HWND__;
struct ID3D11DeviceContext;
struct ID3D11Device;
struct ID3D11Buffer;
struct ID3D11Texture2D;
class Texture;
class D3D;
class Cubemap;
class SkyboxShader;
class RectToCubemapShader;
class FrameCBuffer;
class Camera;
class RenderTexture;

class Skybox
{
public:
	Skybox();
	~Skybox();

	bool Initialise(D3D* d3d, HWND__* hwnd, FrameCBuffer* frameBuffer, Camera* camera);
	bool Render(ID3D11DeviceContext* deviceContext) const;

private:
	bool CreateCubeMap(D3D* d3d, HWND__* hwnd);
	void BindMesh(ID3D11DeviceContext* deviceContext) const;

	ID3D11Buffer* _pVertexBuffer;
	ID3D11Buffer* _pIndexBuffer;
	Cubemap* _pCubeMap;
	Cubemap* _pIrradianceMap;
	Cubemap* _pPreFilterMap;
	RenderTexture* _pBrdfLUT;
	SkyboxShader* _pSkyboxShader;
	FrameCBuffer* _pFrameBuffer;
	Camera* _pCamera;
};
