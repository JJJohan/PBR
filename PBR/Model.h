#pragma once

#include <DirectXMath.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Buffer;

class Model
{
public:
	Model();
	~Model();

	bool Initialise(ID3D11Device* device, DirectX::XMFLOAT3 position);
	void Render(ID3D11DeviceContext*) const;

	DirectX::XMFLOAT3 GetPosition() const;
	int GetIndexCount() const;

private:
	bool InitialiseBuffers(ID3D11Device* device);
	void RenderBuffers(ID3D11DeviceContext* deviceContext) const;

	DirectX::XMFLOAT3 _position;
	ID3D11Buffer* _pVertexBuffer;
	ID3D11Buffer* _pIndexBuffer;
	int _vertexCount;
	int _indexCount;
};
