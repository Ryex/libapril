/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _DIRECTX11
#include <d3d11.h>

#include <hltypes/hlog.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "DirectX11_RenderSystem.h"
#include "DirectX11_VertexShader.h"

#define APRIL_D3D_DEVICE (((DirectX11_RenderSystem*)april::rendersys)->d3dDevice)

using namespace Microsoft::WRL;

namespace april
{
	DirectX11_VertexShader::DirectX11_VertexShader(chstr filename) : VertexShader(), shaderData(NULL), shaderSize(0), dx11Shader(nullptr)
	{
		this->load(filename);
	}

	DirectX11_VertexShader::DirectX11_VertexShader() : VertexShader(), shaderData(NULL), shaderSize(0), dx11Shader(nullptr)
	{
	}

	DirectX11_VertexShader::~DirectX11_VertexShader()
	{
		this->dx11Shader = nullptr;
		_HL_TRY_DELETE_ARRAY(this->shaderData);
	}

	bool DirectX11_VertexShader::load(chstr filename)
	{
		if (this->dx11Shader != nullptr)
		{
			hlog::error(april::logTag, "Shader already loaded.");
			return false;
		}
		if (!this->_loadData(filename, &this->shaderData, &this->shaderSize))
		{
			hlog::error(april::logTag, "Shader file not found: " + filename);
			return false;
		}
		HRESULT hr = APRIL_D3D_DEVICE->CreateVertexShader(this->shaderData, this->shaderSize, NULL, &this->dx11Shader);
		if (FAILED(hr))
		{
			hlog::error(april::logTag, "Failed to create vertex shader!");
			return false;
		}
		return true;
	}

	void DirectX11_VertexShader::setConstantsB(const int* quadVectors, unsigned int quadCount)
	{
	}

	void DirectX11_VertexShader::setConstantsI(const int* quadVectors, unsigned int quadCount)
	{
	}

	void DirectX11_VertexShader::setConstantsF(const float* quadVectors, unsigned int quadCount)
	{
	}

}

#endif