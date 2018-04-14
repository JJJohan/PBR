#pragma once

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

#include <dxgi.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11DepthStencilView;
struct ID3D11Debug;
struct ID3D11RenderTargetView;
struct ID3D11Texture2D;
struct ID3D11DepthStencilState;
struct ID3D11RasterizerState;

class D3D
{
public:
	D3D();
	~D3D();

	bool Initialise(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, float screenDepth,
	                float screenNear);

	void BeginScene(float red, float green, float blue, float alpha) const;
	void EndScene() const;

	ID3D11Device* GetDevice() const;
	ID3D11DeviceContext* GetDeviceContext() const;
	ID3D11DepthStencilView* GetDepthStencilView() const;
	void SetBackBufferRenderTarget();

	void GetVideoCardInfo(char*, int&) const;

	bool ResizeDepthBuffer(int width, int height);
	void ResizeViewport(int width, int height) const;

private:
	bool _vsyncEnabled;
	int _renderWidth;
	int _renderHeight;
	int _videoCardMemory;
	char _videoCardDescription[128];
	IDXGISwapChain* _pSwapChain;
	ID3D11Device* _pDevice;
	ID3D11Debug* _pDebug;
	ID3D11DeviceContext* _pDeviceContext;
	ID3D11RenderTargetView* _pRenderTargetView;
	ID3D11Texture2D* _pDepthStencilBuffer;
	ID3D11DepthStencilState* _pDepthStencilState;
	ID3D11DepthStencilView* _pDepthStencilView;
	ID3D11RasterizerState* _pRasterState;
};
