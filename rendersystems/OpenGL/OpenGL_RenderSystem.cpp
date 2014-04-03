/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 3.32
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _OPENGL

#include <hltypes/hplatform.h>
#if __APPLE__
	#include <TargetConditionals.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if TARGET_OS_IPHONE
	#ifdef _OPENGLES1
		#include <OpenGLES/ES1/gl.h>
		#include <OpenGLES/ES1/glext.h>
	#elif defined(_OPENGLES2)
		#include <OpenGLES/ES2/gl.h>
		#include <OpenGLES/ES2/glext.h>
		extern GLint _positionSlot;
	#endif
#elif defined(_OPENGLES)
	#include <GLES/gl.h>
	#ifdef _ANDROID
		#define GL_GLEXT_PROTOTYPES
		#include <GLES/glext.h>
	#else
		#include <EGL/egl.h>
	#endif
#else
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
#endif

#include <gtypes/Rectangle.h>
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Color.h"
#include "Image.h"
#include "OpenGL_RenderSystem.h"
#include "OpenGL_Texture.h"
#include "Platform.h"
#include "RenderState.h"
#ifdef _WIN32_WINDOW
#include "Win32_Window.h"
#endif

#define MAX_VERTEX_COUNT 65536

#define UINT_RGBA_TO_ABGR(c) (((c & 0xFF000000) >> 24) | ((c & 0x00FF0000) >> 8) | ((c & 0x0000FF00) << 8) | ((c & 0x000000FF) << 24));

namespace april
{
	// TODOa - put in state class
	static Color lastColor = Color::Black;

	// translation from abstract render ops to gl's render ops
	int gl_render_ops[]=
	{
		0,
		GL_TRIANGLES,		// RO_TRIANGLE_LIST
		GL_TRIANGLE_STRIP,	// RO_TRIANGLE_STRIP
		GL_TRIANGLE_FAN,	// RO_TRIANGLE_FAN
		GL_LINES,			// RO_LINE_LIST
		GL_LINE_STRIP,		// RO_LINE_STRIP
		GL_POINTS,			// RO_POINT_LIST
	};
	
	OpenGL_RenderSystem::OpenGL_RenderSystem() : RenderSystem(), activeTexture(NULL)
	{
		this->state = new RenderState(); // TODOa
#if defined(_WIN32) && defined(_WIN32_WINDOW)
		this->hWnd = 0;
		this->hDC = 0;
#endif
	}

	OpenGL_RenderSystem::~OpenGL_RenderSystem()
	{
	}

	bool OpenGL_RenderSystem::create(RenderSystem::Options options)
	{
		if (!RenderSystem::create(options))
		{
			return false;
		}
		this->activeTexture = NULL;
		this->deviceState.reset();
		this->currentState.reset();
		this->state->reset();
		return true;
	}

	bool OpenGL_RenderSystem::destroy()
	{
		if (!RenderSystem::destroy())
		{
			return false;
		}
		this->activeTexture = NULL;
		this->deviceState.reset();
		this->currentState.reset();
		this->state->reset();
#ifdef _WIN32
		this->_releaseWindow();
#endif
		return true;
	}

#ifdef _WIN32
	void OpenGL_RenderSystem::_releaseWindow()
	{
#ifdef _WIN32_WINDOW
		if (april::window->getName() == APRIL_WS_WIN32 && this->hDC != 0)
		{
			ReleaseDC(this->hWnd, this->hDC);
			this->hDC = 0;
		}
#endif
	}

	bool OpenGL_RenderSystem::_initWin32(Window* window)
	{
#ifdef _WIN32_WINDOW
		if (april::window->getName() == APRIL_WS_WIN32)
		{
			this->hWnd = (HWND)window->getBackendId();
			PIXELFORMATDESCRIPTOR pfd;
			memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
			pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
			pfd.nVersion = 1;
			pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |	PFD_DOUBLEBUFFER;
			pfd.iPixelType = PFD_TYPE_RGBA;
			pfd.cColorBits = 24;
			pfd.cStencilBits = 16;
			pfd.dwLayerMask = PFD_MAIN_PLANE;
			this->hDC = GetDC(this->hWnd);
			if (this->hDC == 0)
			{
				hlog::error(april::logTag, "Can't create a GL device context!");
				return false;
			}
			GLuint pixelFormat = ChoosePixelFormat(this->hDC, &pfd);
			if (pixelFormat == 0)
			{
				hlog::error(april::logTag, "Can't find a suitable pixel format!");
				this->_releaseWindow();
				return false;
			}
			if (SetPixelFormat(this->hDC, pixelFormat, &pfd) == 0)
			{
				hlog::error(april::logTag, "Can't set the pixel format!");
				this->_releaseWindow();
				return false;
			}
		}
#endif
		return true;
	}
#endif

	void OpenGL_RenderSystem::assignWindow(Window* window)
	{
		this->_setupDefaultParameters();
		this->setMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		this->setMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		this->orthoProjection.setSize(window->getSize());
	}

	void OpenGL_RenderSystem::reset()
	{
		RenderSystem::reset();
		this->currentState.reset();
		this->deviceState.reset();
		this->_setupDefaultParameters();
		this->currentState.modelviewMatrixChanged = true;
		this->currentState.projectionMatrixChanged = true;
		this->_applyStateChanges();
	}

	void OpenGL_RenderSystem::_setupDefaultParameters()
	{
		glClearColor(0, 0, 0, 1);
		lastColor.set(0, 0, 0, 255);
		this->setViewport(grect(0.0f, 0.0f, april::window->getSize()));
		// GL defaults
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnable(GL_TEXTURE_2D);
		// pixel data
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		// other
		if (this->options.depthBuffer)
		{
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
		}
		this->_setClientState(GL_TEXTURE_COORD_ARRAY, this->deviceState.textureCoordinatesEnabled);
		this->_setClientState(GL_COLOR_ARRAY, this->deviceState.colorEnabled);
		glColor4f(this->deviceState.systemColor.r_f(), this->deviceState.systemColor.g_f(), this->deviceState.systemColor.b_f(), this->deviceState.systemColor.a_f());
		glBindTexture(GL_TEXTURE_2D, this->deviceState.textureId);
		this->currentState.textureFilter = april::Texture::FILTER_NEAREST;
		this->currentState.textureAddressMode = april::Texture::ADDRESS_WRAP;
		this->currentState.blendMode = april::BM_UNDEFINED;
		this->currentState.colorMode = april::CM_UNDEFINED;
	}

	int OpenGL_RenderSystem::getMaxTextureSize()
	{
		int max;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
		return max;
	}

	void OpenGL_RenderSystem::setViewport(grect rect)
	{
		RenderSystem::setViewport(rect);
		// because GL has to defy screen logic and has (0,0) in the bottom left corner
		glViewport((int)rect.x, (int)(april::window->getHeight() - rect.h - rect.y), (int)rect.w, (int)rect.h);
	}
	
	void OpenGL_RenderSystem::clear(bool useColor, bool depth)
	{
		GLbitfield mask = 0;
		if (useColor)
		{
			mask |= GL_COLOR_BUFFER_BIT;
		}
		if (depth)
		{
			mask |= GL_DEPTH_BUFFER_BIT;
		}
		glClear(mask);
	}

	void OpenGL_RenderSystem::clear(bool depth, grect rect, Color color)
	{
		if (color != lastColor) // used to minimize redundant calls to OpenGL
		{
			glClearColor(color.r_f(), color.g_f(), color.b_f(), color.a_f());
			lastColor = color;
		}
		this->clear(true, depth);
	}
	
	void OpenGL_RenderSystem::_applyStateChanges()
	{
		if (this->currentState.textureCoordinatesEnabled != this->deviceState.textureCoordinatesEnabled)
		{
			this->_setClientState(GL_TEXTURE_COORD_ARRAY, this->currentState.textureCoordinatesEnabled);
			this->deviceState.textureCoordinatesEnabled = this->currentState.textureCoordinatesEnabled;
		}
		if (this->currentState.colorEnabled != this->deviceState.colorEnabled)
		{
			this->_setClientState(GL_COLOR_ARRAY, this->currentState.colorEnabled);
			this->deviceState.colorEnabled = this->currentState.colorEnabled;
		}
		if (this->currentState.systemColor != this->deviceState.systemColor)
		{
			glColor4f(this->currentState.systemColor.r_f(), this->currentState.systemColor.g_f(), this->currentState.systemColor.b_f(), this->currentState.systemColor.a_f());
			this->deviceState.systemColor = this->currentState.systemColor;
		}
		if (this->currentState.textureId != this->deviceState.textureId)
		{
			glBindTexture(GL_TEXTURE_2D, this->currentState.textureId);
			this->deviceState.textureId = this->currentState.textureId;
			// TODO - you should memorize address and filter modes per texture in opengl to avoid unnecesarry calls
			this->deviceState.textureAddressMode = Texture::ADDRESS_UNDEFINED;
			this->deviceState.textureFilter = Texture::FILTER_UNDEFINED;
		}
		// texture has to be bound first or else filter and address mode won't be applied afterwards
		if (this->currentState.textureFilter != this->deviceState.textureFilter || this->deviceState.textureFilter == Texture::FILTER_UNDEFINED)
		{
			this->_setTextureFilter(this->currentState.textureFilter);
			this->deviceState.textureFilter = this->currentState.textureFilter;
		}
		if (this->currentState.textureAddressMode != this->deviceState.textureAddressMode || this->deviceState.textureAddressMode == Texture::ADDRESS_UNDEFINED)
		{
			this->_setTextureAddressMode(this->currentState.textureAddressMode);
			this->deviceState.textureAddressMode = this->currentState.textureAddressMode;
		}
		if (this->currentState.blendMode != this->deviceState.blendMode)
		{
			this->_setTextureBlendMode(this->currentState.blendMode);
			this->deviceState.blendMode = this->currentState.blendMode;
		}
		if (this->currentState.colorMode != this->deviceState.colorMode || this->currentState.colorModeFactor != this->deviceState.colorModeFactor)
		{
			this->_setTextureColorMode(this->currentState.colorMode, this->currentState.colorModeFactor);
			this->deviceState.colorMode = this->currentState.colorMode;
			this->deviceState.colorModeFactor = this->currentState.colorModeFactor;
		}
		if (this->currentState.modelviewMatrixChanged && this->modelviewMatrix != this->deviceState.modelviewMatrix)
		{
			this->setMatrixMode(GL_MODELVIEW);
			glLoadMatrixf(this->modelviewMatrix.data);
			this->deviceState.modelviewMatrix = this->modelviewMatrix;
			this->currentState.modelviewMatrixChanged = false;
		}
		if (this->currentState.projectionMatrixChanged && this->projectionMatrix != this->deviceState.projectionMatrix)
		{
			this->setMatrixMode(GL_PROJECTION);
			glLoadMatrixf(this->projectionMatrix.data);
			this->deviceState.projectionMatrix = this->projectionMatrix;
			this->currentState.projectionMatrixChanged = false;
		}
	}

	void OpenGL_RenderSystem::_setClientState(unsigned int type, bool enabled)
	{
		enabled ? glEnableClientState(type) : glDisableClientState(type);
	}

	void OpenGL_RenderSystem::bindTexture(unsigned int textureId)
	{
		this->currentState.textureId = textureId;
	}

	void OpenGL_RenderSystem::setMatrixMode(unsigned int mode)
	{
		// performance call, minimize redundant calls to setMatrixMode
		if (this->deviceState.modeMatrix != mode)
		{
			this->deviceState.modeMatrix = mode;
			glMatrixMode(mode);
		}
	}

	void OpenGL_RenderSystem::setTextureBlendMode(BlendMode mode)
	{
		this->currentState.blendMode = mode;
	}
	
	void OpenGL_RenderSystem::_setTextureBlendMode(BlendMode textureBlendMode)
	{
		if (textureBlendMode == BM_ALPHA || textureBlendMode == BM_DEFAULT)
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else if (textureBlendMode == BM_ADD)
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		}
		else
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			hlog::warn(april::logTag, "Trying to set unsupported blend mode!");
		}
	}
	
	void OpenGL_RenderSystem::setTextureColorMode(ColorMode textureColorMode, float factor)
	{
		this->currentState.colorMode = textureColorMode;
		this->currentState.colorModeFactor = factor;
	}

	void OpenGL_RenderSystem::_setTextureColorMode(ColorMode textureColorMode, float factor)
	{
		static float constColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
		constColor[3] = factor;
		switch (textureColorMode)
		{
		case CM_DEFAULT:
		case CM_MULTIPLY:
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
			break;
		case CM_LERP:
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constColor);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE);
			break;
		case CM_ALPHA_MAP:
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
			break;
		default:
			hlog::warn(april::logTag, "Trying to set unsupported color mode!");
			break;
		}
	}

	void OpenGL_RenderSystem::setTextureFilter(Texture::Filter textureFilter)
	{
		this->currentState.textureFilter = textureFilter;
	}

	void OpenGL_RenderSystem::_setTextureFilter(Texture::Filter textureFilter)
	{
		this->textureFilter = textureFilter;
		switch (textureFilter)
		{
		case Texture::FILTER_LINEAR:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			break;
		case Texture::FILTER_NEAREST:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			break;
		default:
			hlog::warn(april::logTag, "Trying to set unsupported texture filter!");
			break;
		}
	}

	void OpenGL_RenderSystem::setTextureAddressMode(Texture::AddressMode textureAddressMode)
	{
		this->currentState.textureAddressMode = textureAddressMode;
	}

	void OpenGL_RenderSystem::_setTextureAddressMode(Texture::AddressMode textureAddressMode)
	{
		this->textureAddressMode = textureAddressMode;
		switch (textureAddressMode)
		{
		case Texture::ADDRESS_WRAP:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			break;
		case Texture::ADDRESS_CLAMP:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			break;
		default:
			hlog::warn(april::logTag, "Trying to set unsupported texture address mode!");
			break;
		}
	}

	void OpenGL_RenderSystem::setTexture(Texture* texture)
	{
		this->activeTexture = (OpenGL_Texture*)texture;
		if (this->activeTexture == NULL)
		{
			this->bindTexture(0);
		}
		else
		{
			this->setTextureFilter(this->activeTexture->getFilter());
			this->setTextureAddressMode(this->activeTexture->getAddressMode());
			// filtering and wrapping applied before loading texture data, iOS OpenGL guidelines suggest it as an optimization
			this->activeTexture->load();
			this->bindTexture(this->activeTexture->textureId);
		}
	}

	Texture* OpenGL_RenderSystem::getRenderTarget()
	{
		return NULL;
	}
	
	void OpenGL_RenderSystem::setRenderTarget(Texture* texture)
	{
		// TODOa
	}
	
	void OpenGL_RenderSystem::setPixelShader(PixelShader* pixelShader)
	{
		hlog::warn(april::logTag, "Pixel shaders are not implemented!");
	}

	void OpenGL_RenderSystem::setVertexShader(VertexShader* vertexShader)
	{
		hlog::warn(april::logTag, "Vertex shaders are not implemented!");
	}

	PixelShader* OpenGL_RenderSystem::createPixelShader()
	{
		hlog::warn(april::logTag, "Pixel shaders are not implemented!");
		return NULL;
	}

	PixelShader* OpenGL_RenderSystem::createPixelShader(chstr filename)
	{
		hlog::warn(april::logTag, "Pixel shaders are not implemented!");
		return NULL;
	}

	VertexShader* OpenGL_RenderSystem::createVertexShader()
	{
		hlog::warn(april::logTag, "Vertex shaders are not implemented!");
		return NULL;
	}

	VertexShader* OpenGL_RenderSystem::createVertexShader(chstr filename)
	{
		hlog::warn(april::logTag, "Vertex shaders are not implemented!");
		return NULL;
	}

	void OpenGL_RenderSystem::_setTexCoordPointer(int stride, const void *pointer)
	{
		if (this->deviceState.strideTexCoord != stride || this->deviceState.pointerTexCoord != pointer)
		{
			this->deviceState.strideTexCoord = stride;
			this->deviceState.pointerTexCoord = pointer;
			glTexCoordPointer(2, GL_FLOAT, stride, pointer);
		}
	}
	
	void OpenGL_RenderSystem::_setColorPointer(int stride, const void *pointer)
	{
		if (this->deviceState.strideColor != stride || this->deviceState.pointerColor != pointer)
		{
			this->deviceState.strideColor = stride;
			this->deviceState.pointerColor = pointer;
			glColorPointer(4, GL_UNSIGNED_BYTE, stride, pointer);
		}
	}

	void OpenGL_RenderSystem::render(RenderOperation renderOperation, PlainVertex* v, int nVertices)
	{
		this->currentState.textureId = 0;
		this->currentState.textureCoordinatesEnabled = false;
		this->currentState.colorEnabled = false;
		this->currentState.systemColor.set(255, 255, 255, 255);
		this->_applyStateChanges();
		this->_setColorPointer(0, NULL);
		this->_setTexCoordPointer(0, NULL);
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65536 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		int size = nVertices;
#ifdef _ANDROID
		for_iter_step (i, 0, nVertices, size)
		{
			size = this->_limitPrimitives(renderOperation, hmin(nVertices - i, MAX_VERTEX_COUNT));
#endif
			this->_setVertexPointer(sizeof(PlainVertex), v);
			glDrawArrays(gl_render_ops[renderOperation], 0, size);
#ifdef _ANDROID
			v += size;
		}
#endif
	}

	void OpenGL_RenderSystem::render(RenderOperation renderOperation, PlainVertex* v, int nVertices, Color color)
	{
		this->currentState.textureId = 0;
		this->currentState.textureCoordinatesEnabled = false;
		this->currentState.colorEnabled = false;
		this->currentState.systemColor = color;
		this->_applyStateChanges();
		this->_setColorPointer(0, NULL);
		this->_setTexCoordPointer(0, NULL);
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65536 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		int size = nVertices;
#ifdef _ANDROID
		for_iter_step (i, 0, nVertices, size)
		{
			size = this->_limitPrimitives(renderOperation, hmin(nVertices - i, MAX_VERTEX_COUNT));
#endif
			this->_setVertexPointer(sizeof(PlainVertex), v);
			glDrawArrays(gl_render_ops[renderOperation], 0, size);
#ifdef _ANDROID
			v += size;
		}
#endif
	}
	
	void OpenGL_RenderSystem::render(RenderOperation renderOperation, TexturedVertex* v, int nVertices)
	{
		this->currentState.textureCoordinatesEnabled = true;
		this->currentState.colorEnabled = false;
		this->currentState.systemColor.set(255, 255, 255, 255);
		this->_applyStateChanges();
		this->_setColorPointer(0, NULL);
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65536 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		int size = nVertices;
#ifdef _ANDROID
		for_iter_step (i, 0, nVertices, size)
		{
			size = this->_limitPrimitives(renderOperation, hmin(nVertices - i, MAX_VERTEX_COUNT));
#endif
			this->_setVertexPointer(sizeof(TexturedVertex), v);
			this->_setTexCoordPointer(sizeof(TexturedVertex), &v->u);
			glDrawArrays(gl_render_ops[renderOperation], 0, size);
#ifdef _ANDROID
			v += size;
		}
#endif
	}

	void OpenGL_RenderSystem::render(RenderOperation renderOperation, TexturedVertex* v, int nVertices, Color color)
	{
		this->currentState.textureCoordinatesEnabled = true;
		this->currentState.colorEnabled = false;
		this->currentState.systemColor = color;
		this->_applyStateChanges();
		this->_setColorPointer(0, NULL);
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65536 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		int size = nVertices;
#ifdef _ANDROID
		for_iter_step (i, 0, nVertices, size)
		{
			size = this->_limitPrimitives(renderOperation, hmin(nVertices - i, MAX_VERTEX_COUNT));
#endif
			this->_setVertexPointer(sizeof(TexturedVertex), v);
			this->_setTexCoordPointer(sizeof(TexturedVertex), &v->u);
			glDrawArrays(gl_render_ops[renderOperation], 0, size);
#ifdef _ANDROID
			v += size;
		}
#endif
	}

	void OpenGL_RenderSystem::render(RenderOperation renderOperation, ColoredVertex* v, int nVertices)
	{
		this->currentState.textureId = 0;
		this->currentState.textureCoordinatesEnabled = false;
		this->currentState.colorEnabled = true;
		this->currentState.systemColor.set(255, 255, 255, 255);
		for_iter (i, 0, nVertices)
		{
			// making sure this is in AGBR order
			v[i].color = UINT_RGBA_TO_ABGR(v[i].color);
		}
		this->_applyStateChanges();
		this->_setTexCoordPointer(0, NULL);
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65536 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		int size = nVertices;
#ifdef _ANDROID
		for_iter_step (i, 0, nVertices, size)
		{
			size = this->_limitPrimitives(renderOperation, hmin(nVertices - i, MAX_VERTEX_COUNT));
#endif
			this->_setVertexPointer(sizeof(ColoredVertex), v);
			this->_setColorPointer(sizeof(ColoredVertex), &v->color);
			glDrawArrays(gl_render_ops[renderOperation], 0, size);
#ifdef _ANDROID
			v += size;
		}
#endif
	}
	
	void OpenGL_RenderSystem::render(RenderOperation renderOperation, ColoredTexturedVertex* v, int nVertices)
	{
		this->currentState.textureCoordinatesEnabled = true;
		this->currentState.colorEnabled = true;
		this->currentState.systemColor.set(255, 255, 255, 255);
		for_iter (i, 0, nVertices)
		{
			// making sure this is in AGBR order
			v[i].color = UINT_RGBA_TO_ABGR(v[i].color);
		}
		this->_applyStateChanges();
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65536 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		int size = nVertices;
#ifdef _ANDROID
		for_iter_step (i, 0, nVertices, size)
		{
			size = this->_limitPrimitives(renderOperation, hmin(nVertices - i, MAX_VERTEX_COUNT));
#endif
			this->_setVertexPointer(sizeof(ColoredTexturedVertex), v);
			this->_setColorPointer(sizeof(ColoredTexturedVertex), &v->color);
			this->_setTexCoordPointer(sizeof(ColoredTexturedVertex), &v->u);
			glDrawArrays(gl_render_ops[renderOperation], 0, size);
#ifdef _ANDROID
			v += size;
		}
#endif
	}
	
	void OpenGL_RenderSystem::_setModelviewMatrix(const gmat4& matrix)
	{
		this->currentState.modelviewMatrix = matrix;
		this->currentState.modelviewMatrixChanged = true;
	}

	void OpenGL_RenderSystem::_setProjectionMatrix(const gmat4& matrix)
	{
		this->currentState.projectionMatrix = matrix;
		this->currentState.projectionMatrixChanged = true;
	}

	void OpenGL_RenderSystem::_setResolution(int w, int h, bool fullscreen)
	{
		glViewport(0, 0, w, h);
		this->orthoProjection.setSize((float)w, (float)h);
		this->setOrthoProjection(this->orthoProjection);
		this->_applyStateChanges();
	}

	Image::Format OpenGL_RenderSystem::getNativeTextureFormat(Image::Format format)
	{
		switch (format)
		{
		case Image::FORMAT_ARGB:
		case Image::FORMAT_ABGR:
		case Image::FORMAT_RGBA:
			return Image::FORMAT_RGBA;
		case Image::FORMAT_XRGB:
		case Image::FORMAT_RGBX:
		case Image::FORMAT_XBGR:
		case Image::FORMAT_BGRX:
			return Image::FORMAT_RGBX;
		// for optimizations
		case Image::FORMAT_BGRA:
#if !defined(_ANDROID) && !defined(_WIN32)
#ifndef __APPLE__
			return Image::FORMAT_BGRA;
#else
			return Image::FORMAT_BGRA;
#endif
#else
			return Image::FORMAT_RGBA;
#endif
		case Image::FORMAT_RGB:
			return Image::FORMAT_RGB;
			break;
		// for optimizations
		case Image::FORMAT_BGR:
#if !defined(_ANDROID) && !defined(_WIN32)
#ifndef __APPLE__
			return Image::FORMAT_BGR;
#else
			return Image::FORMAT_BGRA;
#endif
#else
			return Image::FORMAT_RGB;
#endif
		case Image::FORMAT_ALPHA:
			return Image::FORMAT_ALPHA;
		case Image::FORMAT_GRAYSCALE:
			return Image::FORMAT_GRAYSCALE;
			break;
		case Image::FORMAT_PALETTE: // TODOaa - does palette use RGBA?
			return Image::FORMAT_PALETTE;
			break;
		}
		return Image::FORMAT_INVALID;
	}

	Image* OpenGL_RenderSystem::takeScreenshot(Image::Format format)
	{
#ifdef _DEBUG
		hlog::write(april::logTag, "Taking screenshot...");
#endif
		int w = april::window->getWidth();
		int h = april::window->getHeight();
		unsigned char* temp = new unsigned char[w * (h + 1) * 4]; // 4 BPP and one extra row just in case some OpenGL implementations don't blit properly and cause a memory leak
		glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, temp);
		unsigned char* data = NULL;
		Image* image = NULL;
		if (Image::convertToFormat(w, h, temp, Image::FORMAT_RGBA, &data, format, false))
		{
			image = Image::create(w, h, data, format);
			delete [] data;
		}
		delete [] temp;
		return image;
	}

}
#endif
