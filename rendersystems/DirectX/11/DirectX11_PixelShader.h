/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a DirectX1 pixel shader.

#ifdef _DIRECTX11
#ifndef APRIL_DIRECTX11_PIXEL_SHADER_H
#define APRIL_DIRECTX11_PIXEL_SHADER_H

#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "PixelShader.h"

using namespace Microsoft::WRL;

namespace april
{
	class DirectX11_RenderSystem;
	
	class DirectX11_PixelShader : public PixelShader
	{
	public:
		friend class DirectX11_RenderSystem;

		DirectX11_PixelShader(chstr filename);
		DirectX11_PixelShader();
		~DirectX11_PixelShader();

		bool load(chstr filename);
		void setConstantsB(const int* quadVectors, unsigned int quadCount);
		void setConstantsI(const int* quadVectors, unsigned int quadCount);
		void setConstantsF(const float* quadVectors, unsigned int quadCount);

	protected:
		ComPtr<ID3D11PixelShader> dx11Shader;

	};

}
#endif
#endif