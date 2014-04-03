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
#include "DirectX11_PixelShader.h"
#include "DirectX11_RenderSystem.h"

#define APRIL_D3D_DEVICE (((DirectX11_RenderSystem*)april::rendersys)->d3dDevice)

using namespace Microsoft::WRL;

namespace april
{
	DirectX11_PixelShader::DirectX11_PixelShader(chstr filename) : PixelShader(), dx11Shader(nullptr)
	{
		this->load(filename);
	}

	DirectX11_PixelShader::DirectX11_PixelShader() : PixelShader(), dx11Shader(nullptr)
	{
	}

	DirectX11_PixelShader::~DirectX11_PixelShader()
	{
		this->dx11Shader = nullptr;
	}

	bool DirectX11_PixelShader::load(chstr filename)
	{
		if (this->dx11Shader != nullptr)
		{
			hlog::error(april::logTag, "Shader already loaded.");
			return false;
		}
		unsigned char* data = NULL;
		long size = 0;
		if (!this->_loadData(filename, &data, &size))
		{
			hlog::error(april::logTag, "Shader file not found: " + filename);
			return false;
		}
		HRESULT hr = APRIL_D3D_DEVICE->CreatePixelShader(data, size, NULL, &this->dx11Shader);
		delete [] data;
		if (FAILED(hr))
		{
			hlog::error(april::logTag, "Failed to create pixel shader!");
			return false;
		}
		return true;
	}

	void DirectX11_PixelShader::setConstantsB(const int* quadVectors, unsigned int quadCount)
	{
	}

	void DirectX11_PixelShader::setConstantsI(const int* quadVectors, unsigned int quadCount)
	{
	}

	void DirectX11_PixelShader::setConstantsF(const float* quadVectors, unsigned int quadCount)
	{
	}

}

#endif