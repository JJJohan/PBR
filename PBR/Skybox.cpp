#include "Skybox.h"
#include "Shapes.h"
#include <dxgiformat.h>
#include "D3D.h"
#include "Graphics.h"
#include "Texture.h"
#include "RenderTexture.h"
#include "Cubemap.h"
#include "SkyboxShader.h"
#include "RectToCubemapShader.h"
#include "FrameCBuffer.h"
#include "Camera.h"
#include "IrradianceShader.h"
#include "PreFilterShader.h"
#include "IntegrateBRDFShader.h"
#include <d3d11.h>

const int SkyboxSize = 2048;
const int IrradianceSize = 32;
const int PreFilterSize = 256;
const int BrdfLookupSize = 512;

const LPCWSTR SkyboxTexture = L"environment.dds";

Skybox::Skybox() = default;

Skybox::~Skybox()
{
	if (_pIndexBuffer)
	{
		_pIndexBuffer->Release();
		_pIndexBuffer = nullptr;
	}

	if (_pVertexBuffer)
	{
		_pVertexBuffer->Release();
		_pVertexBuffer = nullptr;
	}

	if (_pSkyboxShader)
	{
		delete _pSkyboxShader;
		_pSkyboxShader = nullptr;
	}

	if (_pCubeMap)
	{
		delete _pCubeMap;
		_pCubeMap = nullptr;
	}

	if (_pIrradianceMap)
	{
		delete _pIrradianceMap;
		_pIrradianceMap = nullptr;
	}

	if (_pPreFilterMap)
	{
		delete _pPreFilterMap;
		_pPreFilterMap = nullptr;
	}

	if (_pBrdfLUT)
	{
		delete _pBrdfLUT;
		_pBrdfLUT = nullptr;
	}
}

bool Skybox::Initialise(D3D* d3d, const HWND hwnd, FrameCBuffer* frameBuffer, Camera* camera)
{
	ID3D11Device* device = d3d->GetDevice();
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	_pFrameBuffer = frameBuffer;
	_pCamera = camera;

	MeshData meshData;

	int vertexCount, indexCount;
	Shapes::CreateCube(meshData, vertexCount, indexCount);

	PosUvVertexType* vertices = meshData.PosUvVertexData;
	unsigned long* indices = meshData.IndexData;

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(PosUvVertexType) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	HRESULT result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &_pVertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &_pIndexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	_pSkyboxShader = new SkyboxShader;
	if (!_pSkyboxShader->Initialise(device, hwnd))
	{
		return false;
	}

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete[] vertices;
	vertices = nullptr;

	delete[] indices;
	indices = nullptr;

	CreateCubeMap(d3d, hwnd);

	return true;
}

bool Skybox::CreateCubeMap(D3D* d3d, const HWND hwnd)
{
	std::vector<RenderTexture*> cubeFaces;
	ID3D11Device* device = d3d->GetDevice();
	ID3D11DeviceContext* deviceContext = d3d->GetDeviceContext();

	// Load initial image.
	Texture* image = new Texture;
	const HRESULT result = image->Initialise(device, SkyboxTexture);
	if (!result)
	{
		return false;
	}

	// Create faces
	for (int i = 0; i < 6; ++i)
	{
		RenderTexture* renderTexture = new RenderTexture;
		renderTexture->Initialise(device, SkyboxSize, SkyboxSize, 1);
		cubeFaces.push_back(renderTexture);
	}

	RectToCubemapShader* shader = new RectToCubemapShader;
	if (!shader->Initialise(device, hwnd))
	{
		return false;
	}

	BindMesh(deviceContext);

	ID3D11ShaderResourceView* srv = image->GetSRV();
	deviceContext->PSSetShaderResources(0, 1, &srv);

	const float lastFov = _pCamera->GetFOV();
	const float lastAspect = _pCamera->GetAspectRatio();
	_pCamera->SetFOV(90.0f);
	_pCamera->SetAspectRatio(1.0f);

	// Render
	for (int i = 0; i < 6; ++i)
	{
		RenderTexture* texture = cubeFaces[i];

		texture->SetRenderTarget(d3d, deviceContext);
		texture->ClearRenderTarget(deviceContext, d3d->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

		if (i == 0) _pCamera->SetRotation(0.0f, 90.0f, 0.0f); // front
		if (i == 1) _pCamera->SetRotation(0.0f, 270.0f, 0.0f); // back
		if (i == 2) _pCamera->SetRotation(-90.0f, 0.0f, 0.0f); // top
		if (i == 3) _pCamera->SetRotation(90.0f, 0.0f, 0.0f); // bottom
		if (i == 4) _pCamera->SetRotation(0.0f, 0.0f, 0.0f); // left
		if (i == 5) _pCamera->SetRotation(0.0f, 180.0f, 0.0f); // right

		if (!_pCamera->Render(deviceContext, _pFrameBuffer))
		{
			return false;
		}

		if (!shader->Render(deviceContext, 36, _pFrameBuffer))
		{
			return false;
		}
	}

	delete shader;
	delete image;

	_pCubeMap = new Cubemap;
	if (!_pCubeMap->Initialise(device, deviceContext, cubeFaces, SkyboxSize, SkyboxSize, 1))
	{
		return false;
	}

	for (int i = 0; i < 6; ++i)
	{
		delete cubeFaces[i];
		RenderTexture* renderTexture = new RenderTexture;
		renderTexture->Initialise(device, IrradianceSize, IrradianceSize, 1);
		cubeFaces[i] = renderTexture;
	}

	IrradianceShader* irradianceShader = new IrradianceShader;
	if (!irradianceShader->Initialise(device, hwnd))
	{
		return false;
	}

	srv = _pCubeMap->GetSRV();
	deviceContext->PSSetShaderResources(0, 1, &srv);

	// Render
	for (int i = 0; i < 6; ++i)
	{
		RenderTexture* texture = cubeFaces[i];

		texture->SetRenderTarget(d3d, deviceContext);
		texture->ClearRenderTarget(deviceContext, d3d->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

		if (i == 0) _pCamera->SetRotation(0.0f, 90.0f, 0.0f); // front
		if (i == 1) _pCamera->SetRotation(0.0f, 270.0f, 0.0f); // back
		if (i == 2) _pCamera->SetRotation(-90.0f, 0.0f, 0.0f); // top
		if (i == 3) _pCamera->SetRotation(90.0f, 0.0f, 0.0f); // bottom
		if (i == 4) _pCamera->SetRotation(0.0f, 0.0f, 0.0f); // left
		if (i == 5) _pCamera->SetRotation(0.0f, 180.0f, 0.0f); // right

		if (!_pCamera->Render(deviceContext, _pFrameBuffer))
		{
			return false;
		}

		if (!irradianceShader->Render(deviceContext, 36, _pFrameBuffer))
		{
			return false;
		}
	}

	delete irradianceShader;

	_pIrradianceMap = new Cubemap;
	if (!_pIrradianceMap->Initialise(device, deviceContext, cubeFaces, IrradianceSize, IrradianceSize, 1))
	{
		return false;
	}

	PreFilterShader* preFilterShader = new PreFilterShader;
	if (!preFilterShader->Initialise(device, hwnd))
	{
		return false;
	}

	_pPreFilterMap = new Cubemap;
	if (!_pPreFilterMap->Initialise(device, deviceContext, std::vector<RenderTexture*>(), PreFilterSize, PreFilterSize, 5))
	{
		return false;
	}

	// Render
	for (int mip = 0; mip < 5; ++mip)
	{
		const unsigned int mipWidth = unsigned int(PreFilterSize * std::pow(0.5, mip));
		const unsigned int mipHeight = unsigned int(PreFilterSize * std::pow(0.5, mip));

		for (int i = 0; i < 6; ++i)
		{
			delete cubeFaces[i];
			RenderTexture* renderTexture = new RenderTexture;
			renderTexture->Initialise(device, mipWidth, mipHeight, 1);
			cubeFaces[i] = renderTexture;
		}

		const float roughness = float(mip) / 4.0f;
		_pFrameBuffer->SetCustomFloat(0, roughness);

		for (int i = 0; i < 6; ++i)
		{
			RenderTexture* texture = cubeFaces[i];

			texture->SetRenderTarget(d3d, deviceContext);
			texture->ClearRenderTarget(deviceContext, d3d->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

			if (i == 0) _pCamera->SetRotation(0.0f, 90.0f, 0.0f); // front
			if (i == 1) _pCamera->SetRotation(0.0f, 270.0f, 0.0f); // back
			if (i == 2) _pCamera->SetRotation(-90.0f, 0.0f, 0.0f); // top
			if (i == 3) _pCamera->SetRotation(90.0f, 0.0f, 0.0f); // bottom
			if (i == 4) _pCamera->SetRotation(0.0f, 0.0f, 0.0f); // left
			if (i == 5) _pCamera->SetRotation(0.0f, 180.0f, 0.0f); // right

			if (!_pCamera->Render(deviceContext, _pFrameBuffer))
			{
				return false;
			}

			if (!preFilterShader->Render(deviceContext, 36, _pFrameBuffer))
			{
				return false;
			}
		}

		_pPreFilterMap->Copy(deviceContext, cubeFaces, mipWidth, mipHeight, mip);
	}

	delete preFilterShader;

	for (int i = 0; i < 6; ++i)
	{
		delete cubeFaces[i];
	}

	IntegrateBRDFShader* integrateBrdfShader = new IntegrateBRDFShader;
	if (!integrateBrdfShader->Initialise(device, hwnd))
	{
		return false;
	}

	_pBrdfLUT = new RenderTexture;
	if (!_pBrdfLUT->Initialise(device, BrdfLookupSize, BrdfLookupSize, 1))
	{
		return false;
	}

	_pBrdfLUT->SetRenderTarget(d3d, deviceContext);
	_pBrdfLUT->ClearRenderTarget(deviceContext, d3d->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

	_pCamera->SetRotation(0.0f, 0.0f, 0.0f);
	if (!_pCamera->Render(deviceContext, _pFrameBuffer))
	{
		return false;
	}

	if (!integrateBrdfShader->Render(deviceContext, 36, _pFrameBuffer))
	{
		return false;
	}

	delete integrateBrdfShader;

	_pCamera->SetFOV(lastFov);
	_pCamera->SetAspectRatio(lastAspect);

	// Cleanup
	d3d->SetBackBufferRenderTarget();

	return true;
}

void Skybox::BindMesh(ID3D11DeviceContext* deviceContext) const
{
	unsigned int stride = sizeof(PosUvVertexType);
	unsigned int offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &_pVertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool Skybox::Render(ID3D11DeviceContext* deviceContext) const
{
	BindMesh(deviceContext);

	ID3D11ShaderResourceView* texture = _pCubeMap->GetSRV();
	deviceContext->PSSetShaderResources(0, 1, &texture);

	const bool result = _pSkyboxShader->Render(deviceContext, 36, _pFrameBuffer);
	if (!result)
	{
		return false;
	}

	ID3D11ShaderResourceView* irradiance = _pIrradianceMap->GetSRV();
	ID3D11ShaderResourceView* preFilter = _pPreFilterMap->GetSRV();
	ID3D11ShaderResourceView* brdfLut = _pBrdfLUT->GetSRV();

	deviceContext->PSSetShaderResources(0, 1, &irradiance);
	deviceContext->PSSetShaderResources(1, 1, &preFilter);
	deviceContext->PSSetShaderResources(2, 1, &brdfLut);

	return true;
}
