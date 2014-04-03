/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _DIRECTX9
#include <d3d9.h>
#include <d3dx9shader.h>

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "DirectX9_PixelShader.h"
#include "DirectX9_RenderSystem.h"

#define APRIL_D3D_DEVICE (((DirectX9_RenderSystem*)april::rendersys)->d3dDevice)

namespace april
{
	DirectX9_PixelShader::DirectX9_PixelShader(chstr filename) : PixelShader(), dx9Shader(NULL)
	{
		this->load(filename);
	}

	DirectX9_PixelShader::DirectX9_PixelShader() : PixelShader(), dx9Shader(NULL)
	{
	}

	DirectX9_PixelShader::~DirectX9_PixelShader()
	{
		if (this->dx9Shader != NULL)
		{
			this->dx9Shader->Release();
			this->dx9Shader = NULL;
		}
	}

	bool DirectX9_PixelShader::load(chstr filename)
	{
		unsigned char* data = NULL;
		long size = 0;
		if (!this->_loadData(filename, &data, &size))
		{
			hlog::error(april::logTag, "Shader file not found: " + filename);
			return false;
		}
		HRESULT result = APRIL_D3D_DEVICE->CreatePixelShader((DWORD*)data, &this->dx9Shader);
		if (result != D3D_OK)
		{
			hlog::error(april::logTag, "Failed to create pixel shader!");
			return false;
		}
		return true;
	}

	void DirectX9_PixelShader::setConstantsB(const int* quadVectors, unsigned int quadCount)
	{
		for_itert (unsigned int, i, 0, quadCount)
		{
			APRIL_D3D_DEVICE->SetPixelShaderConstantB(i, quadVectors + i * 4, 1);
		}
	}

	void DirectX9_PixelShader::setConstantsI(const int* quadVectors, unsigned int quadCount)
	{
		for_itert (unsigned int, i, 0, quadCount)
		{
			APRIL_D3D_DEVICE->SetPixelShaderConstantI(i, quadVectors + i * 4, 1);
		}
	}

	void DirectX9_PixelShader::setConstantsF(const float* quadVectors, unsigned int quadCount)
	{
		for_itert (unsigned int, i, 0, quadCount)
		{
			APRIL_D3D_DEVICE->SetPixelShaderConstantF(i, quadVectors + i * 4, 1);
		}
	}

}

#endif