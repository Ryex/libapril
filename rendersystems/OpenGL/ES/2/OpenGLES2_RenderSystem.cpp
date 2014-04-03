/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _OPENGLES2
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>

#include "april.h"
#include "OpenGLES2_RenderSystem.h"
#include "OpenGLES2_Texture.h"
#include "Platform.h"

namespace april
{
	OpenGLES2_RenderSystem::OpenGLES2_RenderSystem() : OpenGLES_RenderSystem()
	{
		this->name = APRIL_RS_OPENGLES2;
	}

	OpenGLES2_RenderSystem::~OpenGLES2_RenderSystem()
	{
		this->destroy();
	}

	Texture* OpenGLES2_RenderSystem::_createTexture(bool fromResource)
	{
		return new OpenGLES2_Texture(fromResource);
	}

	void OpenGLES2_RenderSystem::_setVertexPointer(int stride, const void* pointer)
	{
		if (this->deviceState.strideVertex != stride || this->deviceState.pointerVertex != pointer)
		{
			this->deviceState.strideVertex = stride;
			this->deviceState.pointerVertex = pointer;
			glVertexAttribPointer(_positionSlot, 3, GL_FLOAT, GL_FALSE, stride, pointer);
		}
	}
	
}
#endif
