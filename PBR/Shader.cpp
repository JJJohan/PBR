#include "Shader.h"
#include <D3Dcompiler.h>
#include <d3d11.h>
#include <fstream>

Shader::Shader() = default;

Shader::~Shader()
{
	if (_pLayout)
	{
		_pLayout->Release();
		_pLayout = nullptr;
	}

	if (_pPixelShader)
	{
		_pPixelShader->Release();
		_pPixelShader = nullptr;
	}

	if (_pVertexShader)
	{
		_pVertexShader->Release();
		_pVertexShader = nullptr;
	}
}

bool Shader::LoadShader(ID3D11Device* device, const HWND hwnd, const LPCWSTR shaderFileName,
                        D3D11_INPUT_ELEMENT_DESC* inputLayout, const int inputCount)
{
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;

	// Initialize the pointers this function will use to null.
	errorMessage = nullptr;
	vertexShaderBuffer = nullptr;
	pixelShaderBuffer = nullptr;

	// Compile the vertex shader code.
	HRESULT result = D3DCompileFromFile(shaderFileName, nullptr, nullptr, "VSMain", "vs_5_0",
	                                    D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, shaderFileName);
		}
			// If there was nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hwnd, shaderFileName, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// Compile the pixel shader code.
	result = D3DCompileFromFile(shaderFileName, nullptr, nullptr, "PSMain", "ps_5_0",
	                            D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, shaderFileName);
		}
			// If there was  nothing in the error message then it simply could not find the file itself.
		else
		{
			MessageBox(hwnd, shaderFileName, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// Create the vertex shader from the buffer.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(),
	                                    nullptr, &_pVertexShader);
	if (FAILED(result))
	{
		return false;
	}

	// Create the pixel shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), nullptr,
	                                   &_pPixelShader);
	if (FAILED(result))
	{
		return false;
	}

	// Create the vertex input layout.
	result = device->CreateInputLayout(inputLayout, inputCount, vertexShaderBuffer->GetBufferPointer(),
	                                   vertexShaderBuffer->GetBufferSize(), &_pLayout);
	if (FAILED(result))
	{
		return false;
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = nullptr;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = nullptr;

	return true;
}

void Shader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, const HWND hwnd, const LPCWSTR shaderFilename) const
{
	std::ofstream fout;

	// Get a pointer to the error message text buffer.
	char* compileErrors = static_cast<char*>(errorMessage->GetBufferPointer());

	// Get the length of the message.
	const size_t bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for (unsigned long i = 0; i < bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = nullptr;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);
}

void Shader::RenderShader(ID3D11DeviceContext* deviceContext, const int indexCount) const
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(_pLayout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(_pVertexShader, nullptr, 0);
	deviceContext->PSSetShader(_pPixelShader, nullptr, 0);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);
}
