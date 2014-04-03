/// @file
/// @author  Boris Mikic
/// @version 3.32
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <gtypes/Matrix4.h>

#include "Color.h"
#include "RenderState.h"
#include "RenderSystem.h"
#include "Texture.h"

namespace april
{
	RenderState::RenderState()
	{
		this->reset();
	}
	
	RenderState::~RenderState()
	{
	}
	
	void RenderState::reset()
	{
		this->textureCoordinatesEnabled = false;
		this->colorEnabled = false;
		this->textureId = 0;
		this->textureFilter = Texture::FILTER_UNDEFINED;
		this->textureAddressMode = Texture::ADDRESS_UNDEFINED;
		this->systemColor = Color::Black;
		this->modelviewMatrixChanged = false;
		this->projectionMatrixChanged = false;
		this->blendMode = BM_UNDEFINED;
		this->colorMode = CM_UNDEFINED;
		this->colorModeFactor = 1.0f;
		this->modeMatrix = 0;
		this->strideVertex = 0;
		this->pointerVertex = NULL;
		this->strideTexCoord = 0;
		this->pointerTexCoord = NULL;
		this->strideColor = 0;
		this->pointerColor = NULL;
	}

}
