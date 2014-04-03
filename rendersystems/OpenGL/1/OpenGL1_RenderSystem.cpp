/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _OPENGL1
#include <hltypes/hplatform.h>
#if __APPLE__
#include <TargetConditionals.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef __APPLE__
#include <gl/GL.h>
#define GL_GLEXT_PROTOTYPES
#include <gl/glext.h>
#else
#include <OpenGL/gl.h>
#endif

#include <gtypes/Vector2.h>
#include <hltypes/exception.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>

#include "april.h"
#include "Image.h"
#include "Keys.h"
#include "OpenGL1_RenderSystem.h"
#include "OpenGL1_Texture.h"
#include "Platform.h"
#include "Timer.h"
#include "Window.h"

namespace april
{
	OpenGL1_RenderSystem::OpenGL1_RenderSystem() : OpenGL_RenderSystem()
	{
		this->name = APRIL_RS_OPENGL1;
#ifdef _WIN32
		this->hRC = 0;
#endif
	}

	OpenGL1_RenderSystem::~OpenGL1_RenderSystem()
	{
		this->destroy();
	}

#ifdef _WIN32
	void OpenGL1_RenderSystem::_releaseWindow()
	{
		if (this->hRC != 0)
		{
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(this->hRC);
			this->hRC = 0;
		}
		OpenGL_RenderSystem::_releaseWindow();
	}
#endif

	void OpenGL1_RenderSystem::assignWindow(Window* window)
	{
#ifdef _WIN32
		if (!this->_initWin32(window))
		{
			return;
		}
		if (april::window->getName() == APRIL_WS_WIN32)
		{
			this->hRC = wglCreateContext(this->hDC);
			if (this->hRC == 0)
			{
				hlog::error(april::logTag, "Can't create a GL rendering context!");
				this->_releaseWindow();
				return;
			}
			if (wglMakeCurrent(this->hDC, this->hRC) == 0)
			{
				hlog::error(april::logTag, "Can't activate the GL rendering context!");
				this->_releaseWindow();
				return;
			}
		}
#endif
		OpenGL_RenderSystem::assignWindow(window);
	}

	void OpenGL1_RenderSystem::_setupDefaultParameters()
	{
		OpenGL_RenderSystem::_setupDefaultParameters();
		// pixel data
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
	}

	int OpenGL1_RenderSystem::getMaxTextureSize()
	{
#ifdef _WIN32
		if (this->hRC == 0)
		{
			return 0;
		}
#endif
		return OpenGL_RenderSystem::getMaxTextureSize();
	}

	void OpenGL1_RenderSystem::_setTextureBlendMode(BlendMode textureBlendMode)
	{
		// TODO - is there a way to make this work on Win32?
#ifndef _WIN32
		// TODO - refactor
		static int blendSeparationSupported = -1;
		if (blendSeparationSupported == -1)
		{
			// determine if blend separation is possible on first call to this function
			hstr extensions = (const char*)glGetString(GL_EXTENSIONS);
			blendSeparationSupported = extensions.contains("GL_EXT_blend_equation_separate") && extensions.contains("GL_EXT_blend_func_separate");
		}
		if (blendSeparationSupported)
		{
			// blending for the new generations
			switch (textureBlendMode)
			{
			case BM_DEFAULT:
			case BM_ALPHA:
				glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case BM_ADD:
				glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case BM_SUBTRACT:
				glBlendEquationSeparate(GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_ADD);
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case BM_OVERWRITE:
				glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
				glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
				break;
			default:
				hlog::warn(april::logTag, "Trying to set unsupported blend mode!");
				break;
			}
		}
		else
#endif
		{
			// old-school blending mode for your dad
			OpenGL_RenderSystem::_setTextureBlendMode(textureBlendMode);
		}
	}
	
	Texture* OpenGL1_RenderSystem::_createTexture(bool fromResource)
	{
		return new OpenGL1_Texture(fromResource);
	}

	void OpenGL1_RenderSystem::_setVertexPointer(int stride, const void* pointer)
	{
		if (this->deviceState.strideVertex != stride || this->deviceState.pointerVertex != pointer)
		{
			this->deviceState.strideVertex = stride;
			this->deviceState.pointerVertex = pointer;
			glVertexPointer(3, GL_FLOAT, stride, pointer);
		}
	}
	
}

#endif
