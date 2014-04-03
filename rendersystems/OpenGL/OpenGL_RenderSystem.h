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
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic OpenGL render system.

#ifdef _OPENGL
#ifndef APRIL_OPENGL_RENDER_SYSTEM_H
#define APRIL_OPENGL_RENDER_SYSTEM_H

#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "OpenGL_State.h"
#include "RenderSystem.h"

namespace april
{
	class OpenGL_Texture;
	class Window;

	class OpenGL_RenderSystem : public RenderSystem
	{
	public:
		friend class OpenGL_Texture;

		OpenGL_RenderSystem();
		~OpenGL_RenderSystem();
		bool create(Options options);
		bool destroy();

		void reset();
		void assignWindow(Window* window);

		float getPixelOffset() { return 0.0f; }
		int getMaxTextureSize();
		void setViewport(grect value);

		void clear(bool useColor = true, bool depth = false);
		void clear(bool depth, grect rect, Color color = Color::Clear);

		void bindTexture(unsigned int textureId);

		void setMatrixMode(unsigned int mode);
		void setTexture(Texture* texture);
		Texture* getRenderTarget();
		void setRenderTarget(Texture* texture);
		void setPixelShader(PixelShader* pixelShader);
		void setVertexShader(VertexShader* vertexShader);
		void setTextureBlendMode(BlendMode mode);
		/// @note The parameter factor is only used when the color mode is LERP.
		void setTextureColorMode(ColorMode textureColorMode, float factor = 1.0f);
		void setTextureFilter(Texture::Filter textureFilter);
		void setTextureAddressMode(Texture::AddressMode textureAddressMode);

		PixelShader* createPixelShader();
		PixelShader* createPixelShader(chstr filename);
		VertexShader* createVertexShader();
		VertexShader* createVertexShader(chstr filename);

		void render(RenderOperation renderOperation, PlainVertex* v, int nVertices);
		void render(RenderOperation renderOperation, PlainVertex* v, int nVertices, Color color);
		void render(RenderOperation renderOperation, TexturedVertex* v, int nVertices);
		void render(RenderOperation renderOperation, TexturedVertex* v, int nVertices, Color color);
		void render(RenderOperation renderOperation, ColoredVertex* v, int nVertices);
		void render(RenderOperation renderOperation, ColoredTexturedVertex* v, int nVertices);
		
		Image::Format getNativeTextureFormat(Image::Format format);
		Image* takeScreenshot(Image::Format format);

	protected:
		OpenGL_State deviceState;
		OpenGL_State currentState;
		OpenGL_Texture* activeTexture;

		virtual void _setupDefaultParameters();
		virtual void _applyStateChanges();
		void _setClientState(unsigned int type, bool enabled);
		void _setModelviewMatrix(const gmat4& matrix);
		void _setProjectionMatrix(const gmat4& matrix);

		void _setResolution(int w, int h, bool fullscreen);

		virtual void _setTextureBlendMode(BlendMode textureBlendMode);
		/// @note The parameter factor is only used when the color mode is LERP.
		virtual void _setTextureColorMode(ColorMode textureColorMode, float factor = 1.0f);
		virtual void _setTextureFilter(Texture::Filter textureFilter);
		virtual void _setTextureAddressMode(Texture::AddressMode textureAddressMode);

		virtual void _setVertexPointer(int stride, const void* pointer) = 0;
		virtual void _setTexCoordPointer(int stride, const void* pointer);
		virtual void _setColorPointer(int stride, const void* pointer);

#ifdef _WIN32
	public:
		HDC getHDC() { return this->hDC; }
		bool _initWin32(Window* window);

	protected:
		HWND hWnd;
		HDC hDC;

		void _releaseWindow();

#endif

	};
	
}
#endif
#endif
