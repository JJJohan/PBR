#pragma once

#include <DirectXMath.h>

struct ID3D11DeviceContext;
class Input;
class FrameCBuffer;

class Camera
{
public:
	Camera();
	~Camera();

	void Initialise(int width, int height, float camNear, float camFar);
	void SetPosition(float x, float y, float z);
	void SetRotation(float pitch, float yaw, float roll);

	DirectX::XMFLOAT3 GetPosition() const;
	DirectX::XMFLOAT3 GetRotation() const;

	void SetFOV(float degrees);
	void SetAspectRatio(float aspectRatio);
	void SetNearFar(float camNear, float camFar);

	float GetFOV() const;
	float GetAspectRatio() const;

	bool Render(ID3D11DeviceContext* deviceContext, FrameCBuffer* frameBuffer);
	void GetViewMatrix(DirectX::XMMATRIX& viewMatrix) const;
	void GetProjectionMatrix(DirectX::XMMATRIX& projectionMatrix) const;
	void GetWorldMatrix(DirectX::XMMATRIX& worldMatrix) const;
	void UpdateInput(Input* input);

private:
	DirectX::XMFLOAT3 _position;
	DirectX::XMFLOAT3 _eulerAngles;
	float _camNear;
	float _camFar;
	float _aspectRatio;
	float _fov;

	DirectX::XMMATRIX _viewMatrix;
	DirectX::XMMATRIX _rotationMatrix;
	DirectX::XMMATRIX _projectionMatrix;
	DirectX::XMMATRIX _worldMatrix;
};
