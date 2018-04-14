#include "D3D.h"
#include <d3d11.h>

D3D::D3D() = default;

D3D::~D3D()
{
	if (_pSwapChain)
	{
		_pSwapChain->SetFullscreenState(false, nullptr);
	}

	if (_pRasterState)
	{
		_pRasterState->Release();
		_pRasterState = nullptr;
	}

	if (_pDepthStencilView)
	{
		_pDepthStencilView->Release();
		_pDepthStencilView = nullptr;
	}

	if (_pDepthStencilState)
	{
		_pDepthStencilState->Release();
		_pDepthStencilState = nullptr;
	}

	if (_pDepthStencilBuffer)
	{
		_pDepthStencilBuffer->Release();
		_pDepthStencilBuffer = nullptr;
	}

	if (_pRenderTargetView)
	{
		_pRenderTargetView->Release();
		_pRenderTargetView = nullptr;
	}

	if (_pDebug)
	{
		_pDebug->Release();
		_pDebug = nullptr;
	}

	if (_pDeviceContext)
	{
		_pDeviceContext->Release();
		_pDeviceContext = nullptr;
	}

	if (_pDevice)
	{
		_pDevice->Release();
		_pDevice = nullptr;
	}

	if (_pSwapChain)
	{
		_pSwapChain->Release();
		_pSwapChain = nullptr;
	}
}

bool D3D::Initialise(const int screenWidth, const int screenHeight, const bool vsync, const HWND hwnd,
                     const bool fullscreen, const float screenDepth, const float screenNear)
{
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	unsigned int numModes, numerator = 0, denominator = 0;
	size_t stringLength;
	DXGI_ADAPTER_DESC adapterDesc;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11Texture2D* backBufferPtr;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_RASTERIZER_DESC rasterDesc;

	_renderWidth = screenWidth;
	_renderHeight = screenHeight;

	// Store the vsync setting.
	_vsyncEnabled = vsync;

	// Create a DirectX graphics interface factory.
	HRESULT result = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&factory));
	if (FAILED(result))
	{
		return false;
	}

	// Use the factory to create an adapter for the primary graphics interface (video card).
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		return false;
	}

	// Enumerate the primary adapter output (monitor).
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return false;
	}

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_ENUM_MODES_INTERLACED, &numModes,
	                                           nullptr);
	if (FAILED(result))
	{
		return false;
	}

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	DXGI_MODE_DESC* displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		return false;
	}

	// Now fill the display mode list structures.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_ENUM_MODES_INTERLACED, &numModes,
	                                           displayModeList);
	if (FAILED(result))
	{
		return false;
	}

	// Now go through all the display modes and find the one that matches the screen width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	for (unsigned int i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Width == static_cast<unsigned int>(screenWidth))
		{
			if (displayModeList[i].Height == static_cast<unsigned int>(screenHeight))
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	// Get the adapter (video card) description.
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}

	// Store the dedicated video card memory in megabytes.
	_videoCardMemory = int(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// Convert the name of the video card to a character array and store it.
	const int error = wcstombs_s(&stringLength, _videoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
	{
		return false;
	}

	// Release the display mode list.
	delete[] displayModeList;
	displayModeList = nullptr;

	// Release the adapter output.
	adapterOutput->Release();
	adapterOutput = nullptr;

	// Release the adapter.
	adapter->Release();
	adapter = nullptr;

	// Release the factory.
	factory->Release();
	factory = nullptr;

	// Initialize the swap chain description.
	ZeroMemory(&swapChainDesc, sizeof swapChainDesc);
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;
	//swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Set the refresh rate of the back buffer.
	if (_vsyncEnabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = !fullscreen;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	// Set the feature level to DirectX 11.
	featureLevel = D3D_FEATURE_LEVEL_11_0;

	UINT creationFlags = 0;

#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Create the swap chain, Direct3D device, and Direct3D device context.
	result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, &featureLevel, 1,
	                                       D3D11_SDK_VERSION, &swapChainDesc, &_pSwapChain, &_pDevice, nullptr,
	                                       &_pDeviceContext);
	if (FAILED(result))
	{
		return false;
	}

#ifdef _DEBUG
	result = _pDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&_pDebug));
	if (FAILED(result))
	{
		return false;
	}
#endif

	// Get the pointer to the back buffer.
	result = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&backBufferPtr));
	if (FAILED(result))
	{
		return false;
	}

	// Create the render target view with the back buffer pointer.
	result = _pDevice->CreateRenderTargetView(backBufferPtr, nullptr, &_pRenderTargetView);
	if (FAILED(result))
	{
		return false;
	}

	// Release pointer to the back buffer as we no longer need it.
	backBufferPtr->Release();
	backBufferPtr = nullptr;

	if (!ResizeDepthBuffer(screenWidth, screenHeight))
	{
		return false;
	}

	// Initialize the description of the stencil state.
	ZeroMemory(&depthStencilDesc, sizeof depthStencilDesc);

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state.
	result = _pDevice->CreateDepthStencilState(&depthStencilDesc, &_pDepthStencilState);
	if (FAILED(result))
	{
		return false;
	}

	// Set the depth stencil state.
	_pDeviceContext->OMSetDepthStencilState(_pDepthStencilState, 1);

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	_pDeviceContext->OMSetRenderTargets(1, &_pRenderTargetView, _pDepthStencilView);

	// Setup the raster description which will determine how and what polygons will be drawn.
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	result = _pDevice->CreateRasterizerState(&rasterDesc, &_pRasterState);
	if (FAILED(result))
	{
		return false;
	}

	// Now set the rasterizer state.
	_pDeviceContext->RSSetState(_pRasterState);

	ResizeViewport(screenWidth, screenHeight);

	return true;
}

void D3D::ResizeViewport(const int width, const int height) const
{
	D3D11_VIEWPORT viewport;

	viewport.Width = float(width);
	viewport.Height = float(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	// Create the viewport.
	_pDeviceContext->RSSetViewports(1, &viewport);
}

bool D3D::ResizeDepthBuffer(const int width, const int height)
{
	if (_pDepthStencilBuffer)
	{
		D3D11_TEXTURE2D_DESC currentDesc;
		_pDepthStencilBuffer->GetDesc(&currentDesc);

		if (currentDesc.Width == width && currentDesc.Height == height)
		{
			return true;
		}

		_pDepthStencilBuffer->Release();
	}

	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof depthBufferDesc);

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = width;
	depthBufferDesc.Height = height;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	HRESULT result = _pDevice->CreateTexture2D(&depthBufferDesc, nullptr, &_pDepthStencilBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Set the depth stencil state.
	_pDeviceContext->OMSetDepthStencilState(_pDepthStencilState, 1);

	// Initailze the depth stencil view.
	ZeroMemory(&depthStencilViewDesc, sizeof depthStencilViewDesc);

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	if (_pDepthStencilView)
	{
		_pDepthStencilView->Release();
	}

	// Create the depth stencil view.
	result = _pDevice->CreateDepthStencilView(_pDepthStencilBuffer, &depthStencilViewDesc, &_pDepthStencilView);
	return !FAILED(result);
}

void D3D::BeginScene(const float red, const float green, const float blue, const float alpha) const
{
	float color[4];

	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	_pDeviceContext->ClearRenderTargetView(_pRenderTargetView, color);
	_pDeviceContext->ClearDepthStencilView(_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

ID3D11DepthStencilView* D3D::GetDepthStencilView() const
{
	return _pDepthStencilView;
}

void D3D::SetBackBufferRenderTarget()
{
	if (!ResizeDepthBuffer(_renderWidth, _renderHeight))
	{
		return;
	}

	ResizeViewport(_renderWidth, _renderHeight);
	_pDeviceContext->OMSetRenderTargets(1, &_pRenderTargetView, _pDepthStencilView);
}

void D3D::EndScene() const
{
	if (_vsyncEnabled)
	{
		// Lock to screen refresh rate.
		_pSwapChain->Present(1, 0);
	}
	else
	{
		// Present as fast as possible.
		_pSwapChain->Present(0, 0);
	}
}

ID3D11Device* D3D::GetDevice() const
{
	return _pDevice;
}

ID3D11DeviceContext* D3D::GetDeviceContext() const
{
	return _pDeviceContext;
}

void D3D::GetVideoCardInfo(char* cardName, int& memory) const
{
	strcpy_s(cardName, 128, _videoCardDescription);
	memory = _videoCardMemory;
}
