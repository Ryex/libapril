/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.33
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a DirectX render system.

#ifdef _DIRECTX9
#ifndef APRIL_DIRECTX9_RENDER_SYSTEM_H
#define APRIL_DIRECTX9_RENDER_SYSTEM_H

#include <hltypes/hplatform.h>

#include "DirectX_RenderSystem.h"

struct _D3DPRESENT_PARAMETERS_;
struct IDirect3D9;
struct IDirect3DDevice9;
struct IDirect3DSurface9;

namespace april
{
	class DirectX9_PixelShader;
	class DirectX9_Texture;
	class DirectX9_VertexShader;
	class Image;
	class Window;

	class DirectX9_RenderSystem : public DirectX_RenderSystem
	{
	public:
		friend class DirectX9_PixelShader;
		friend class DirectX9_Texture;
		friend class DirectX9_VertexShader;

		DirectX9_RenderSystem();
		~DirectX9_RenderSystem();
		bool create(Options options);
		bool destroy();

		void reset();
		void assignWindow(Window* window);

		float getPixelOffset() { return 0.5f; }
		int getMaxTextureSize();
		harray<DisplayMode> getSupportedDisplayModes();
		void setViewport(grect rect);

		void setTextureBlendMode(BlendMode textureBlendMode);
		/// @note The parameter factor is only used when the color mode is LERP.
		void setTextureColorMode(ColorMode textureColorMode, float factor = 1.0f);
		void setTextureFilter(Texture::Filter textureFilter);
		void setTextureAddressMode(Texture::AddressMode textureAddressMode);
		void setTexture(Texture* texture);
		Texture* getRenderTarget();
		void setRenderTarget(Texture* source);
		void setPixelShader(PixelShader* pixelShader);
		void setVertexShader(VertexShader* vertexShader);

		PixelShader* createPixelShader();
		PixelShader* createPixelShader(chstr filename);
		VertexShader* createVertexShader();
		VertexShader* createVertexShader(chstr filename);

		void clear(bool useColor = true, bool depth = false);
		void clear(bool depth, grect rect, Color color = Color::Clear);
		void render(RenderOperation renderOperation, PlainVertex* v, int nVertices);
		void render(RenderOperation renderOperation, PlainVertex* v, int nVertices, Color color);
		void render(RenderOperation renderOperation, TexturedVertex* v, int nVertices);
		void render(RenderOperation renderOperation, TexturedVertex* v, int nVertices, Color color);
		void render(RenderOperation renderOperation, ColoredVertex* v, int nVertices);
		void render(RenderOperation renderOperation, ColoredTexturedVertex* v, int nVertices);

		Image::Format getNativeTextureFormat(Image::Format format);
		Image* takeScreenshot(Image::Format format);
		void presentFrame();

	protected:
		bool textureCoordinatesEnabled;
		bool colorEnabled;
		IDirect3D9* d3d;
		IDirect3DDevice9* d3dDevice;
		DirectX9_Texture* activeTexture;
		DirectX9_Texture* renderTarget;
		IDirect3DSurface9* backBuffer;
		harray<DisplayMode> supportedDisplayModes;
		_D3DPRESENT_PARAMETERS_* d3dpp;
		HWND childHWnd;

		void _configureDevice();
		void _setResolution(int w, int h, bool fullscreen);

		void _tryAssignChildWindow();
		void _tryUnassignChildWindow();

		Texture* _createTexture(bool fromResource);

		void _setModelviewMatrix(const gmat4& matrix);
		void _setProjectionMatrix(const gmat4& matrix);
		
	private:
		bool _supportsA8Surface; // this does not seem to be detectable via any type of device caps

	};

}
#endif
#endif
