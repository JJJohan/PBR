#include "Graphics.h"
#include "Input.h"
#include "D3D.h"
#include "Camera.h"
#include "Model.h"
#include "PBRShader.h"
#include "Skybox.h"
#include "FrameCBuffer.h"
#include "ObjectCBuffer.h"
#include "Texture.h"
#include <d3d11.h>

Graphics::Graphics() = default;

Graphics::~Graphics()
{
	if (_pPBRShader)
	{
		delete _pPBRShader;
		_pPBRShader = nullptr;
	}

	for (auto it = _pModels.begin(); it != _pModels.end(); ++it)
	{
		delete *it;
	}
	_pModels.clear();

	if (_pCamera)
	{
		delete _pCamera;
		_pCamera = nullptr;
	}

	if (_pSkybox)
	{
		delete _pSkybox;
		_pSkybox = nullptr;
	}

	if (_pMetallic)
	{
		delete _pMetallic;
		_pMetallic = nullptr;
	}

	if (_pRoughness)
	{
		delete _pRoughness;
		_pRoughness = nullptr;
	}

	if (_pNormal)
	{
		delete _pNormal;
		_pNormal = nullptr;
	}

	if (_pFrameBuffer)
	{
		delete _pFrameBuffer;
		_pFrameBuffer = nullptr;
	}

	if (_pObjectBuffer)
	{
		delete _pObjectBuffer;
		_pObjectBuffer = nullptr;
	}

	if (_pD3D)
	{
		delete _pD3D;
		_pD3D = nullptr;
	}
}

bool Graphics::Initialise(const int screenWidth, const int screenHeight, const HWND hwnd, Input* input)
{
	_pInput = input;

	// Create the Direct3D object.
	_pD3D = new D3D;
	if (!_pD3D)
	{
		return false;
	}

	// Initialize the Direct3D object.
	bool result = _pD3D->Initialise(screenWidth, screenHeight, VsyncEnabled, hwnd, FullScreen, ScreenDepth, ScreenNear);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

	ID3D11Device* device = _pD3D->GetDevice();

	_pMetallic = new Texture;
	HRESULT hr = _pMetallic->Initialise(device, L"metallic.dds");
	if (!hr)
	{
		return false;
	}

	_pNormal = new Texture;
	hr = _pNormal->Initialise(device, L"normal.dds");
	if (!hr)
	{
		return false;
	}

	_pRoughness = new Texture;
	hr = _pRoughness->Initialise(device, L"roughness.dds");
	if (!hr)
	{
		return false;
	}

	// Create the camera object.
	_pCamera = new Camera;
	_pCamera->Initialise(screenWidth, screenHeight, ScreenNear, ScreenDepth);

	// Set the initial position of the camera.
	_pCamera->SetPosition(0.0f, 0.0f, -10.0f);

	_pFrameBuffer = new FrameCBuffer;
	_pFrameBuffer->Initialise(device);

	_pObjectBuffer = new ObjectCBuffer;
	_pObjectBuffer->Initialise(device);

	_pSkybox = new Skybox;
	_pSkybox->Initialise(_pD3D, hwnd, _pFrameBuffer, _pCamera);

	// Create the model object.
	for (int i = 0; i < 10; ++i)
	{
		for (int j = 0; j < 10; ++j)
		{
			Model* model = new Model;

			result = model->Initialise(device, XMFLOAT3(i * 2.0f, j * 2.0f, 0.0f));
			if (!result)
			{
				MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
				return false;
			}

			_pModels.push_back(model);
		}
	}

	_pPBRShader = new PBRShader;
	if (!_pPBRShader)
	{
		return false;
	}

	result = _pPBRShader->Initialise(device, hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the PBR shader object.", L"Error", MB_OK);
		return false;
	}

	return true;
}

bool Graphics::Frame() const
{
	_pCamera->UpdateInput(_pInput);

	return true;
}

bool Graphics::Render() const
{
	ID3D11DeviceContext* context = _pD3D->GetDeviceContext();
	XMMATRIX worldMatrix;

	_pD3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	bool result = _pCamera->Render(context, _pFrameBuffer);
	if (!result)
	{
		return false;
	}

	result = _pSkybox->Render(context);
	if (!result)
	{
		return false;
	}

	_pCamera->GetWorldMatrix(worldMatrix);

	// Bind textures.
	ID3D11ShaderResourceView* normal = _pNormal->GetSRV();
	ID3D11ShaderResourceView* roughness = _pRoughness->GetSRV();
	ID3D11ShaderResourceView* metallic = _pMetallic->GetSRV();
	context->PSSetShaderResources(3, 1, &normal);
	context->PSSetShaderResources(4, 1, &roughness);
	context->PSSetShaderResources(5, 1, &metallic);

	// Render meshes.
	for (auto it = _pModels.begin(); it != _pModels.end(); ++it)
	{
		Model* model = *it;

		model->Render(context);

		result = _pObjectBuffer->Update(context, worldMatrix, model->GetPosition());
		if (!result)
		{
			return false;
		}

		result = _pPBRShader->Render(context, model->GetIndexCount(), _pFrameBuffer, _pObjectBuffer);
		if (!result)
		{
			return false;
		}
	}

	_pD3D->EndScene();
	return true;
}
