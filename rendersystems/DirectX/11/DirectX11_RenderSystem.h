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
/// Defines a DirectX11 render system.

#ifdef _DIRECTX11
#ifndef APRIL_DIRECTX11_RENDER_SYSTEM_H
#define APRIL_DIRECTX11_RENDER_SYSTEM_H

#ifndef _WINP8
#include <windows.ui.xaml.media.dxinterop.h>
using namespace Windows::UI::Xaml::Controls;
#endif

#include <d3d11_1.h>

#include <gtypes/Matrix4.h>
#include <gtypes/Quaternion.h>
#include <gtypes/Rectangle.h>
#include <hltypes/harray.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "RenderSystem.h"
#include "Window.h"

using namespace Microsoft::WRL;
using namespace Windows::UI::Core;

namespace april
{
	class DirectX11_PixelShader;
	class DirectX11_Texture;
	class DirectX11_VertexShader;
	class Window;

	class DirectX11_RenderSystem : public RenderSystem
	{
	public:
		friend class DirectX11_PixelShader;
		friend class DirectX11_Texture;
		friend class DirectX11_VertexShader;

		struct ConstantBuffer
		{
			gmat4 matrix;
			gquat lerpAlpha; // must be, because of 16 byte alignment of constant buffer size
		};

		DirectX11_RenderSystem();
		~DirectX11_RenderSystem();
		bool create(Options options);
		bool destroy();

		void assignWindow(Window* window);
		void reset();

		float getPixelOffset() { return 0.0f; }
		int getMaxTextureSize();
		harray<DisplayMode> getSupportedDisplayModes();
		void setViewport(grect value);

		void setTextureBlendMode(BlendMode textureBlendMode);
		void setTextureColorMode(ColorMode textureColorMode, unsigned char alpha = 255);
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
		void render(RenderOp renderOp, PlainVertex* v, int nVertices);
		void render(RenderOp renderOp, PlainVertex* v, int nVertices, Color color);
		void render(RenderOp renderOp, TexturedVertex* v, int nVertices);
		void render(RenderOp renderOp, TexturedVertex* v, int nVertices, Color color);
		void render(RenderOp renderOp, ColoredVertex* v, int nVertices);
		void render(RenderOp renderOp, ColoredTexturedVertex* v, int nVertices);

		Image* takeScreenshot(int bpp = 3);
		void presentFrame();

		void updateOrientation();

	protected:
		BlendMode activeTextureBlendMode;
		ColorMode activeTextureColorMode;
		unsigned char activeTextureColorModeAlpha;
		DirectX11_Texture* activeTexture;
		DirectX11_VertexShader* activeVertexShader;
		DirectX11_PixelShader* activePixelShader;
		DirectX11_Texture* renderTarget;
		harray<DisplayMode> supportedDisplayModes;
		grect viewport;

		DirectX11_VertexShader* vertexShaderDefault;
		DirectX11_PixelShader* pixelShaderTexturedMultiply;
		DirectX11_PixelShader* pixelShaderTexturedAlphaMap;
		DirectX11_PixelShader* pixelShaderTexturedLerp;
		DirectX11_PixelShader* pixelShaderMultiply;
		DirectX11_PixelShader* pixelShaderAlphaMap;
		DirectX11_PixelShader* pixelShaderLerp;

		void _createSwapChain(int width, int height);
		void _resizeSwapChain(int width, int height);
		void _configureSwapChain();
		void _configureDevice();
		
		void _setResolution(int w, int h, bool fullscreen);

		Texture* _createTexture(chstr filename);
		Texture* _createTexture(int w, int h, unsigned char* rgba);
		Texture* _createTexture(int w, int h, Texture::Format format, Texture::Type type = Texture::TYPE_NORMAL, Color color = Color::Clear);

		void _setModelviewMatrix(const gmat4& matrix);
		void _setProjectionMatrix(const gmat4& matrix);
		void _updatePixelShader(bool useTexture);
		void _updateVertexShader();

	private:
		ComPtr<ID3D11Device1> d3dDevice;
		ComPtr<ID3D11DeviceContext1> d3dDeviceContext;
		ComPtr<IDXGISwapChain1> swapChain;
#ifndef _WINP8
		ComPtr<ISwapChainBackgroundPanelNative>	swapChainNative;
#endif

		ComPtr<ID3D11RasterizerState> rasterState;
		ComPtr<ID3D11RenderTargetView> renderTargetView;
		ComPtr<ID3D11BlendState> blendStateAlpha;
		ComPtr<ID3D11BlendState> blendStateAdd;
		ComPtr<ID3D11BlendState> blendStateSubtract;
		ComPtr<ID3D11BlendState> blendStateOverwrite;
		ComPtr<ID3D11SamplerState> samplerLinearWrap;
		ComPtr<ID3D11SamplerState> samplerLinearClamp;
		ComPtr<ID3D11SamplerState> samplerNearestWrap;
		ComPtr<ID3D11SamplerState> samplerNearestClamp;

		D3D11_BUFFER_DESC vertexBufferDesc;
		D3D11_SUBRESOURCE_DATA vertexBufferData;
		D3D11_MAPPED_SUBRESOURCE mappedSubResource;
		ComPtr<ID3D11Buffer> vertexBuffer;

		ComPtr<ID3D11Buffer> constantBuffer;
		ConstantBuffer constantBufferData;

		ComPtr<ID3D11InputLayout> inputLayout;

		DirectX11_VertexShader* _currentVertexShader;
		DirectX11_PixelShader* _currentPixelShader;
		DirectX11_Texture* _currentTexture;
		BlendMode _currentTextureBlendMode;
		ColorMode _currentTextureColorMode;
		Texture::Filter _currentTextureFilter;
		Texture::AddressMode _currentTextureAddressMode;
		RenderOp _currentRenderOp;
		ID3D11Buffer** _currentVertexBuffer;
		
		bool matrixDirty;

		void _setRenderOp(RenderOp renderOp);
		void _updateVertexBuffer(int nVertices, void* data);
		void _updateConstantBuffer();
		void _updateBlendMode();
		void _updateTexture(bool use);

	};

}
#endif
#endif
