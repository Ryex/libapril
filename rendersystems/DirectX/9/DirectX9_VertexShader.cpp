/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _DIRECTX9
#include <d3dx9shader.h>

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "DirectX9_RenderSystem.h"
#include "DirectX9_VertexShader.h"

#define APRIL_D3D_DEVICE (((DirectX9_RenderSystem*)april::rendersys)->d3dDevice)

namespace april
{
	DirectX9_VertexShader::DirectX9_VertexShader(chstr filename) : VertexShader(), dx9Shader(NULL)
	{
		this->load(filename);
	}

	DirectX9_VertexShader::DirectX9_VertexShader() : VertexShader(), dx9Shader(NULL)
	{
	}

	DirectX9_VertexShader::~DirectX9_VertexShader()
	{
		if (this->dx9Shader != NULL)
		{
			this->dx9Shader->Release();
			this->dx9Shader = NULL;
		}
	}

	bool DirectX9_VertexShader::load(chstr filename)
	{
		unsigned char* data = NULL;
		long size = 0;
		if (!this->_loadData(filename, &data, &size))
		{
			hlog::error(april::logTag, "Shader file not found: " + filename);
			return false;
		}
		HRESULT result = APRIL_D3D_DEVICE->CreateVertexShader((DWORD*)data, &this->dx9Shader);
		if (result != D3D_OK)
		{
			hlog::error(april::logTag, "Failed to create vertex shader!");
			return false;
		}
		return true;
	}

	void DirectX9_VertexShader::setConstantsB(const int* quadVectors, unsigned int quadCount)
	{
		for_itert (unsigned int, i, 0, quadCount)
		{
			APRIL_D3D_DEVICE->SetVertexShaderConstantB(i, quadVectors + i * 4, 1);
		}
	}

	void DirectX9_VertexShader::setConstantsI(const int* quadVectors, unsigned int quadCount)
	{
		for_itert (unsigned int, i, 0, quadCount)
		{
			APRIL_D3D_DEVICE->SetVertexShaderConstantI(i, quadVectors + i * 4, 1);
		}
	}

	void DirectX9_VertexShader::setConstantsF(const float* quadVectors, unsigned int quadCount)
	{
		for_itert (unsigned int, i, 0, quadCount)
		{
			APRIL_D3D_DEVICE->SetVertexShaderConstantF(i, quadVectors + i * 4, 1);
		}
	}

}

#endif