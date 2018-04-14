#include "Camera.h"
#include <cmath>
#include "Input.h"
#include <d3d11.h>
#include "FrameCBuffer.h"

using namespace DirectX;

Camera::Camera()
{
	_position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	_eulerAngles = XMFLOAT3(0.0f, 0.0f, 0.0f);
	_aspectRatio = 1.0f;
	_fov = XMConvertToRadians(45.0f);
	_camNear = 0.1f;
	_camFar = 1000.0f;
	_worldMatrix = XMMatrixIdentity();
}

Camera::~Camera()
{
}

void Camera::Initialise(const int width, const int height, const float camNear, const float camFar)
{
	const float screenAspect = float(width) / float(height);

	SetAspectRatio(screenAspect);
	SetNearFar(camNear, camFar);
}

void Camera::SetAspectRatio(const float aspectRatio)
{
	_aspectRatio = aspectRatio;
	_projectionMatrix = XMMatrixPerspectiveFovLH(_fov, _aspectRatio, _camNear, _camFar);
}

void Camera::SetNearFar(const float camNear, const float camFar)
{
	_camNear = camNear;
	_camFar = camFar;
	_projectionMatrix = XMMatrixPerspectiveFovLH(_fov, _aspectRatio, _camNear, _camFar);
}

void Camera::SetFOV(const float degrees)
{
	_fov = XMConvertToRadians(degrees);
	_projectionMatrix = XMMatrixPerspectiveFovLH(_fov, _aspectRatio, _camNear, _camFar);
}

float Camera::GetFOV() const
{
	return XMConvertToDegrees(_fov);
}

float Camera::GetAspectRatio() const
{
	return _aspectRatio;
}

void Camera::SetPosition(const float x, const float y, const float z)
{
	_position.x = x;
	_position.y = y;
	_position.z = z;
}

void Camera::SetRotation(const float pitch, const float yaw, const float roll)
{
	_eulerAngles.x = pitch;
	_eulerAngles.y = yaw;
	_eulerAngles.z = roll;
}

XMFLOAT3 Camera::GetPosition() const
{
	return XMFLOAT3(_position);
}

XMFLOAT3 Camera::GetRotation() const
{
	return XMFLOAT3(_eulerAngles);
}

bool Camera::Render(ID3D11DeviceContext* deviceContext, FrameCBuffer* frameBuffer)
{
	// Setup the vector that points upwards.
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	// Setup the position of the camera in the world.
	const XMVECTOR position = XMLoadFloat3(&_position);

	// Setup where the camera is looking by default.
	XMVECTOR lookAt = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
	const float pitch = XMConvertToRadians(_eulerAngles.x);
	const float yaw = XMConvertToRadians(_eulerAngles.y);
	const float roll = XMConvertToRadians(_eulerAngles.z);

	// Create the rotation matrix from the yaw, pitch, and roll values.
	_rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	// Transform the lookAt and up vector by the rotation matrix so the view is correctly rotated at the origin.
	lookAt = XMVector3TransformCoord(lookAt, _rotationMatrix);
	up = XMVector3TransformCoord(up, _rotationMatrix);

	// Translate the rotated camera position to the location of the viewer.
	lookAt = XMVectorAdd(position, lookAt);

	// Finally create the view matrix from the three updated vectors.
	_viewMatrix = XMMatrixLookAtLH(position, lookAt, up);

	const HRESULT result = frameBuffer->Update(deviceContext, _viewMatrix, _projectionMatrix, GetPosition());
	return !FAILED(result);
}

void Camera::GetViewMatrix(XMMATRIX& viewMatrix) const
{
	viewMatrix = _viewMatrix;
}

void Camera::GetProjectionMatrix(XMMATRIX& projectionMatrix) const
{
	projectionMatrix = _projectionMatrix;
}

void Camera::GetWorldMatrix(XMMATRIX& worldMatrix) const
{
	worldMatrix = _worldMatrix;
}

void Camera::UpdateInput(Input* input)
{
	int x, y;
	input->GetMouseDelta(x, y);

	const float sensitivity = 0.1f;
	const float xScaled = x * sensitivity;
	const float yScaled = y * sensitivity;

	_eulerAngles.x += yScaled;
	_eulerAngles.y += xScaled;

	float forwardScale = 0.0f;
	float sideScale = 0.0f;

	const float movementScale = 0.1f;

	if (input->IsKeyDown(DIK_W))
	{
		forwardScale += movementScale;
	}
	if (input->IsKeyDown(DIK_S))
	{
		forwardScale -= movementScale;
	}
	if (input->IsKeyDown(DIK_A))
	{
		sideScale -= movementScale;
	}
	if (input->IsKeyDown(DIK_D))
	{
		sideScale += movementScale;
	}

	if (std::fabs(forwardScale) + std::fabs(sideScale) > 0.0f)
	{
		const XMVECTOR position = XMLoadFloat3(&_position);
		XMVECTOR translation = XMVector3TransformCoord(XMVectorSet(sideScale, 0.0f, forwardScale, 0.0f), _rotationMatrix);
		translation = XMVectorAdd(translation, position);

		XMStoreFloat3(&_position, translation);
	}
}
