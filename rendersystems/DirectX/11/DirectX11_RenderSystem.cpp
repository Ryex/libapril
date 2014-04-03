/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _DIRECTX11
#include <d3d11_1.h>
#include <stdio.h>

#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/exception.h>
#include <hltypes/hlog.h>
#include <hltypes/hplatform.h>
#include <hltypes/hresource.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "DirectX11_PixelShader.h"
#include "DirectX11_RenderSystem.h"
#include "DirectX11_Texture.h"
#include "DirectX11_VertexShader.h"
#include "Image.h"
#include "Keys.h"
#include "Platform.h"
#include "RenderState.h"
#include "Timer.h"
#include "WinRT.h"
#include "WinRT_Window.h"

#define SHADER_PATH "april/"
#define VERTICES_BUFFER_COUNT 65536
#ifndef _WINP8
#define BACKBUFFER_COUNT 2
#else
#define BACKBUFFER_COUNT 1
#endif

#define UINT_RGBA_TO_ABGR(c) ((((c) >> 24) & 0xFF) | (((c) << 24) & 0xFF000000) | (((c) >> 8) & 0xFF00) | (((c) << 8) & 0xFF0000))

#ifndef _WINP8
#define _LOAD_SHADER(name, type, file) \
	if (name == NULL) \
	{ \
		name = (DirectX11_ ## type ## Shader*)this->create ## type ## Shader(SHADER_PATH #type "Shader_" #file ".cso"); \
	}
#else // on WinP8 the shaders may be in the root directory
#define _LOAD_SHADER(name, type, file) \
	if (name == NULL) \
	{ \
		hstr filename = #type "Shader_" #file ".cso"; \
		if (hresource::exists(SHADER_PATH + filename)) \
		{ \
			name = (DirectX11_ ## type ## Shader*)this->create ## type ## Shader(SHADER_PATH + filename); \
		} \
		else \
		{ \
			name = (DirectX11_ ## type ## Shader*)this->create ## type ## Shader(filename); \
		} \
	}
#endif

using namespace Microsoft::WRL;
using namespace Windows::Graphics::Display;

namespace april
{
	static ColoredTexturedVertex static_ctv[VERTICES_BUFFER_COUNT];

	// TODO - refactor
	harray<DirectX11_Texture*> gRenderTargets;

	D3D11_PRIMITIVE_TOPOLOGY dx11_render_ops[]=
	{
		D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED,
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,	// ROP_TRIANGLE_LIST
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,	// ROP_TRIANGLE_STRIP
		D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED,		// triangle fans are deprecated in DX11
		D3D11_PRIMITIVE_TOPOLOGY_LINELIST,		// ROP_LINE_LIST
		D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,		// ROP_LINE_STRIP
		D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,		// ROP_POINT_LIST
	};

	unsigned int _numPrimitives(RenderOp renderOp, int nVertices)
	{
		switch (renderOp)
		{
		case TriangleList:
			return nVertices / 3;
		case TriangleStrip:
			return nVertices - 2;
		case TriangleFan:
			return nVertices - 1;
		case LineList:
			return nVertices / 2;
		case LineStrip:
			return nVertices - 1;
		case PointList:
			return nVertices;
		}
		return 0;
	}
	
	DirectX11_RenderSystem::DirectX11_RenderSystem() : RenderSystem(),
		activeTextureBlendMode(DEFAULT), activeTexture(NULL), renderTarget(NULL),
		activeTextureColorMode(NORMAL), activeTextureColorModeAlpha(255),
		matrixDirty(true)
	{
		this->name = APRIL_RS_DIRECTX11;
		this->state = new RenderState(); // TODOa
		this->d3dDevice = nullptr;
		this->d3dDeviceContext = nullptr;
		this->swapChain = nullptr;
#ifndef _WINP8
		this->swapChainNative = nullptr;
#endif
		this->rasterState = nullptr;
		this->renderTargetView = nullptr;
		this->blendStateAlpha = nullptr;
		this->blendStateAdd = nullptr;
		this->blendStateSubtract = nullptr;
		this->blendStateOverwrite = nullptr;
		this->samplerLinearWrap = nullptr;
		this->samplerLinearClamp = nullptr;
		this->samplerNearestWrap = nullptr;
		this->samplerNearestClamp = nullptr;
		this->vertexBuffer = nullptr;
		this->constantBuffer = nullptr;
		this->inputLayout = nullptr;
		this->vertexShaderDefault = NULL;
		this->pixelShaderTexturedMultiply = NULL;
		this->pixelShaderTexturedAlphaMap = NULL;
		this->pixelShaderTexturedLerp = NULL;
		this->pixelShaderMultiply = NULL;
		this->pixelShaderAlphaMap = NULL;
		this->pixelShaderLerp = NULL;
		this->_currentVertexShader = NULL;
		this->_currentPixelShader = NULL;
		this->_currentTexture = NULL;
		this->_currentTextureBlendMode = BLEND_MODE_UNDEFINED;
		this->_currentTextureColorMode = COLOR_MODE_UNDEFINED;
		this->_currentTextureFilter = Texture::FILTER_UNDEFINED;
		this->_currentTextureAddressMode = Texture::ADDRESS_UNDEFINED;
		this->_currentRenderOp = RENDER_OP_UNDEFINED;
		this->_currentVertexBuffer = NULL;
	}

	DirectX11_RenderSystem::~DirectX11_RenderSystem()
	{
		this->destroy();
	}

	bool DirectX11_RenderSystem::create(Options options)
	{
		if (!RenderSystem::create(options))
		{
			return false;
		}
		this->activeTextureBlendMode = DEFAULT;
		this->activeTexture = NULL;
		this->renderTarget = NULL;
		this->activeTextureColorMode = NORMAL;
		this->activeTextureColorModeAlpha = 255;
		this->matrixDirty = true;
		this->d3dDevice = nullptr;
		this->d3dDeviceContext = nullptr;
		this->swapChain = nullptr;
#ifndef _WINP8
		this->swapChainNative = nullptr;
#endif
		this->rasterState = nullptr;
		this->renderTargetView = nullptr;
		this->blendStateAlpha = nullptr;
		this->blendStateAdd = nullptr;
		this->blendStateSubtract = nullptr;
		this->blendStateOverwrite = nullptr;
		this->samplerLinearWrap = nullptr;
		this->samplerLinearClamp = nullptr;
		this->samplerNearestWrap = nullptr;
		this->samplerNearestClamp = nullptr;
		this->vertexBuffer = nullptr;
		this->constantBuffer = nullptr;
		this->inputLayout = nullptr;
		this->vertexShaderDefault = NULL;
		this->pixelShaderTexturedMultiply = NULL;
		this->pixelShaderTexturedAlphaMap = NULL;
		this->pixelShaderTexturedLerp = NULL;
		this->pixelShaderMultiply = NULL;
		this->pixelShaderAlphaMap = NULL;
		this->pixelShaderLerp = NULL;
		this->_currentVertexShader = NULL;
		this->_currentPixelShader = NULL;
		this->_currentTexture = NULL;
		this->_currentTextureBlendMode = BLEND_MODE_UNDEFINED;
		this->_currentTextureColorMode = COLOR_MODE_UNDEFINED;
		this->_currentTextureFilter = Texture::FILTER_UNDEFINED;
		this->_currentTextureAddressMode = Texture::ADDRESS_UNDEFINED;
		this->_currentRenderOp = RENDER_OP_UNDEFINED;
		this->_currentVertexBuffer = NULL;
		this->viewport.setSize(april::getSystemInfo().displayResolution);
		return true;
	}

	bool DirectX11_RenderSystem::destroy()
	{
		if (!RenderSystem::destroy())
		{
			return false;
		}
		_HL_TRY_DELETE(this->vertexShaderDefault);
		_HL_TRY_DELETE(this->pixelShaderTexturedMultiply);
		_HL_TRY_DELETE(this->pixelShaderTexturedAlphaMap);
		_HL_TRY_DELETE(this->pixelShaderTexturedLerp);
		_HL_TRY_DELETE(this->pixelShaderMultiply);
		_HL_TRY_DELETE(this->pixelShaderAlphaMap);
		_HL_TRY_DELETE(this->pixelShaderLerp);
		this->inputLayout = nullptr;
		this->vertexBuffer = nullptr;
		this->constantBuffer = nullptr;
		this->samplerLinearWrap = nullptr;
		this->samplerLinearClamp = nullptr;
		this->samplerNearestWrap = nullptr;
		this->samplerNearestClamp = nullptr;
		this->blendStateAlpha = nullptr;
		this->blendStateAdd = nullptr;
		this->blendStateSubtract = nullptr;
		this->blendStateOverwrite = nullptr;
		this->renderTargetView = nullptr;
		this->rasterState = nullptr;
#ifndef _WINP8
		this->swapChainNative = nullptr;
#endif
		this->swapChain = nullptr;
		this->d3dDeviceContext = nullptr;
		this->d3dDevice = nullptr;
		this->_currentVertexShader = NULL;
		this->_currentPixelShader = NULL;
		this->_currentTexture = NULL;
		this->_currentTextureBlendMode = BLEND_MODE_UNDEFINED;
		this->_currentTextureColorMode = COLOR_MODE_UNDEFINED;
		this->_currentTextureFilter = Texture::FILTER_UNDEFINED;
		this->_currentTextureAddressMode = Texture::ADDRESS_UNDEFINED;
		this->_currentRenderOp = RENDER_OP_UNDEFINED;
		this->_currentVertexBuffer = NULL;
		this->viewport.setSize(april::getSystemInfo().displayResolution);
		return true;
	}

	void DirectX11_RenderSystem::assignWindow(Window* window)
	{
		unsigned int creationFlags = 0;
#if !defined(_DEBUG) || defined(_WINP8)
		creationFlags |= D3D11_CREATE_DEVICE_PREVENT_ALTERING_LAYER_SETTINGS_FROM_REGISTRY;
#else
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_DEBUGGABLE;
#endif
		D3D_FEATURE_LEVEL featureLevels[] =
		{
#ifndef _WINP8
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1
#else
			D3D_FEATURE_LEVEL_9_3
#endif
		};
		ComPtr<ID3D11Device> _d3dDevice;
		ComPtr<ID3D11DeviceContext> _d3dDeviceContext;
		HRESULT hr;
		hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, featureLevels,
			ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &_d3dDevice, NULL, &_d3dDeviceContext);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to create DX11 device!");
		}
		hr = _d3dDevice.As(&this->d3dDevice);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to retrieve Direct3D 11.1 device interface!");
		}
		hr = _d3dDeviceContext.As(&this->d3dDeviceContext);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to retrieve Direct3D 11.1 device context interface!");
		}
		// device config
		this->_configureDevice();
		// initial vertex buffer data
		this->vertexBufferData.pSysMem = NULL;
		this->vertexBufferData.SysMemPitch = 0;
		this->vertexBufferData.SysMemSlicePitch = 0;
		this->vertexBufferDesc.ByteWidth = 0;
		this->vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		this->vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		this->vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		this->vertexBufferDesc.MiscFlags = 0;
		this->vertexBufferDesc.StructureByteStride = 0;
		// initial constant buffer
		D3D11_SUBRESOURCE_DATA constantSubresourceData = {0};
		constantSubresourceData.pSysMem = &this->constantBufferData;
		constantSubresourceData.SysMemPitch = 0;
		constantSubresourceData.SysMemSlicePitch = 0;
		D3D11_BUFFER_DESC constantBufferDesc = {0};
		constantBufferDesc.ByteWidth = sizeof(this->constantBufferData);
		constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		constantBufferDesc.MiscFlags = 0;
		constantBufferDesc.StructureByteStride = 0;
		hr = this->d3dDevice->CreateBuffer(&constantBufferDesc, &constantSubresourceData, &this->constantBuffer);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to create constant buffer!");
		}
		this->d3dDeviceContext->VSSetConstantBuffers(0, 1, this->constantBuffer.GetAddressOf());
		this->matrixDirty = true;
		// initial calls
		this->clear();
		this->presentFrame();
		this->orthoProjection.setSize((float)window->getWidth(), (float)window->getHeight());
		// default shaders
		_LOAD_SHADER(this->vertexShaderDefault, Vertex, Default);
		_LOAD_SHADER(this->pixelShaderTexturedMultiply, Pixel, TexturedMultiply);
		_LOAD_SHADER(this->pixelShaderTexturedAlphaMap, Pixel, TexturedAlphaMap);
		_LOAD_SHADER(this->pixelShaderTexturedLerp, Pixel, TexturedLerp);
		_LOAD_SHADER(this->pixelShaderMultiply, Pixel, Multiply);
		_LOAD_SHADER(this->pixelShaderAlphaMap, Pixel, AlphaMap);
		_LOAD_SHADER(this->pixelShaderLerp, Pixel, Lerp);
		// input layouts for default shaders
		if (this->inputLayout == nullptr)
		{
			const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
			{
				{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
			};
			hr = this->d3dDevice->CreateInputLayout(inputLayoutDesc, ARRAYSIZE(inputLayoutDesc),
				this->vertexShaderDefault->shaderData, this->vertexShaderDefault->shaderSize, &this->inputLayout);
			if (FAILED(hr))
			{
				throw hl_exception("Unable to create input layout for colored-textured shader!");
			}
		}
		this->d3dDeviceContext->IASetInputLayout(this->inputLayout.Get());
	}

	void DirectX11_RenderSystem::reset()
	{
		RenderSystem::reset();
#ifndef _WINP8
		// possible Microsoft bug, required for SwapChainBackgroundPanel to update its layout 
		IInspectable* panelInspectable = (IInspectable*)reinterpret_cast<IInspectable*>(WinRT::XamlOverlay);
		panelInspectable->QueryInterface(__uuidof(ISwapChainBackgroundPanelNative), (void**)&this->swapChainNative);
		this->swapChainNative->SetSwapChain(this->swapChain.Get());
#endif
	}

	void DirectX11_RenderSystem::_createSwapChain(int width, int height)
	{
		// Once the swap chain desc is configured, it must be
		// created on the same adapter as the existing D3D Device.
		HRESULT hr;
		ComPtr<IDXGIDevice2> dxgiDevice;
		hr = this->d3dDevice.As(&dxgiDevice);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to retrieve DXGI device!");
		}
		hr = dxgiDevice->SetMaximumFrameLatency(1);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to set MaximumFrameLatency!");
		}
		ComPtr<IDXGIAdapter> dxgiAdapter;
		hr = dxgiDevice->GetAdapter(&dxgiAdapter);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to get adapter from DXGI device!");
		}
		ComPtr<IDXGIFactory2> dxgiFactory;
		hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
		if (FAILED(hr))
		{
			throw hl_exception("Unable to get parent factory from DXGI adapter!");
		}
		gvec2 resolution = april::getSystemInfo().displayResolution;
		int w = hround(resolution.x);
		int h = hround(resolution.y);
		if (w != width || h != height)
		{
			hlog::warnf(april::logTag, "On WinRT the window resolution (%d,%d) should match the display resolution (%d,%d) in order to avoid problems.", width, height, w, h);
		}
		CHECK_SWAP(width, height);
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
		swapChainDesc.Stereo = false;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		swapChainDesc.Width = width;
		swapChainDesc.Height = height;
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferCount = BACKBUFFER_COUNT;
#ifndef _WINP8
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		hr = dxgiFactory->CreateSwapChainForComposition(this->d3dDevice.Get(), &swapChainDesc, nullptr, &this->swapChain);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to create swap chain!");
		}
		IInspectable* panelInspectable = (IInspectable*)reinterpret_cast<IInspectable*>(WinRT::XamlOverlay);
		panelInspectable->QueryInterface(__uuidof(ISwapChainBackgroundPanelNative), (void**)&this->swapChainNative);
		this->swapChainNative->SetSwapChain(this->swapChain.Get());
#else
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		hr = dxgiFactory->CreateSwapChainForCoreWindow(this->d3dDevice.Get(),
			reinterpret_cast<IUnknown*>(Windows::UI::Core::CoreWindow::GetForCurrentThread()), &swapChainDesc, NULL, &this->swapChain);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to create swap chain!");
		}
#endif
		this->_configureSwapChain();
		this->updateOrientation();
	}

	void DirectX11_RenderSystem::_resizeSwapChain(int width, int height)
	{
		this->d3dDeviceContext->OMSetRenderTargets(0, NULL, NULL);
		this->renderTargetView = nullptr;
		HRESULT hr = this->swapChain->ResizeBuffers(BACKBUFFER_COUNT, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to resize swap chain buffers!");
		}
		this->_configureSwapChain();
		this->updateOrientation();
	}

	void DirectX11_RenderSystem::_configureSwapChain()
	{
		ComPtr<ID3D11Texture2D> _backBuffer;
		HRESULT hr = this->swapChain->GetBuffer(0, IID_PPV_ARGS(&_backBuffer));
		if (FAILED(hr))
		{
			throw hl_exception("Unable to get swap chain back buffer!");
		}
		// Create a descriptor for the RenderTargetView.
		CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(D3D11_RTV_DIMENSION_TEXTURE2DARRAY, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 0, 1);
		hr = this->d3dDevice->CreateRenderTargetView(_backBuffer.Get(), &renderTargetViewDesc, &this->renderTargetView);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to create render target view!");
		}
		// has to use GetAddressOf(), because the parameter is a pointer to an array of render target views
		this->d3dDeviceContext->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), NULL);
	}

	void DirectX11_RenderSystem::_configureDevice()
	{
		if (this->swapChain != nullptr)
		{
			this->_resizeSwapChain(april::window->getWidth(), april::window->getHeight());
		}
		else
		{
			this->_createSwapChain(april::window->getWidth(), april::window->getHeight());
		}
		ComPtr<ID3D11Texture2D> _backBuffer;
		HRESULT hr = this->swapChain->GetBuffer(0, IID_PPV_ARGS(&_backBuffer));
		D3D11_RASTERIZER_DESC rasterDesc;
		rasterDesc.AntialiasedLineEnable = false;
		rasterDesc.CullMode = D3D11_CULL_NONE;
		rasterDesc.DepthBias = 0;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.DepthClipEnable = true;
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.FrontCounterClockwise = false;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.ScissorEnable = false;
		rasterDesc.SlopeScaledDepthBias = 0.0f;
		hr = this->d3dDevice->CreateRasterizerState(&rasterDesc, &this->rasterState);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to create raster state!");
		}
		this->d3dDeviceContext->RSSetState(this->rasterState.Get());
		D3D11_TEXTURE2D_DESC backBufferDesc = {0};
		_backBuffer->GetDesc(&backBufferDesc);
		CHECK_SWAP(backBufferDesc.Width, backBufferDesc.Height);
		this->setViewport(grect(0.0f, 0.0f, (float)backBufferDesc.Width, (float)backBufferDesc.Height)); // just to be on the safe side and prevent floating point errors
		// blend modes
		D3D11_BLEND_DESC blendDesc = {0};
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = false;
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = (D3D11_COLOR_WRITE_ENABLE_RED |
			D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE);
		// alpha
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		this->d3dDevice->CreateBlendState(&blendDesc, &this->blendStateAlpha);
		// add
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		this->d3dDevice->CreateBlendState(&blendDesc, &this->blendStateAdd);
		// subtract
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		this->d3dDevice->CreateBlendState(&blendDesc, &this->blendStateSubtract);
		// overwrite
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
		this->d3dDevice->CreateBlendState(&blendDesc, &this->blendStateOverwrite);
		// texture samplers
		D3D11_SAMPLER_DESC samplerDesc;
		memset(&samplerDesc, 0, sizeof(samplerDesc));
		samplerDesc.MaxAnisotropy = 0;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.BorderColor[0] = 0.0f;
		samplerDesc.BorderColor[1] = 0.0f;
		samplerDesc.BorderColor[2] = 0.0f;
		samplerDesc.BorderColor[3] = 0.0f;
		// linear + wrap
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		this->d3dDevice->CreateSamplerState(&samplerDesc, &this->samplerLinearWrap);
		// linear + clamp
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		this->d3dDevice->CreateSamplerState(&samplerDesc, &this->samplerLinearClamp);
		// nearest neighbor + wrap
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		this->d3dDevice->CreateSamplerState(&samplerDesc, &this->samplerNearestWrap);
		// nearest neighbor + clamp
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		this->d3dDevice->CreateSamplerState(&samplerDesc, &this->samplerNearestClamp);
		// other
		this->setTextureBlendMode(DEFAULT);
		this->setTextureColorMode(NORMAL);
		this->setTextureAddressMode(Texture::ADDRESS_WRAP);
		this->setTextureFilter(Texture::FILTER_LINEAR);
		this->clear();
		this->presentFrame();
	}

	void DirectX11_RenderSystem::updateOrientation()
	{
		DisplayOrientations orientation = DisplayProperties::CurrentOrientation;
		DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;
		switch (orientation)
		{
		case DisplayOrientations::Landscape:
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			break;
		case DisplayOrientations::Portrait:
			rotation = DXGI_MODE_ROTATION_IDENTITY;
			break;
		case DisplayOrientations::LandscapeFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE270;
			break;
		case DisplayOrientations::PortraitFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE180;
			break;
		default:
			hlog::error(april::logTag, "Undefined screen orienation, using default landscape!");
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			break;
		}
	}

	void DirectX11_RenderSystem::_setResolution(int w, int h, bool fullscreen)
	{
		if (this->swapChain != nullptr)
		{
			this->_resizeSwapChain(april::window->getWidth(), april::window->getHeight());
		}
		else
		{
			this->_createSwapChain(april::window->getWidth(), april::window->getHeight());
		}
		this->matrixDirty = true;
	}

	int DirectX11_RenderSystem::getMaxTextureSize()
	{
		// depends on FEATURE_LEVEL, while 9.3 supports 4096, 9.2 and 9.1 support only 2048 so using 2048 is considered safe
		return D3D_FL9_1_REQ_TEXTURE1D_U_DIMENSION;
	}

	harray<DisplayMode> DirectX11_RenderSystem::getSupportedDisplayModes()
	{
		if (this->supportedDisplayModes.size() == 0)
		{
			gvec2 resolution = april::getSystemInfo().displayResolution;
			this->supportedDisplayModes += DisplayMode((int)resolution.x, (int)resolution.y, 60);
		}
		return this->supportedDisplayModes;
	}

	void DirectX11_RenderSystem::setViewport(grect value)
	{
		RenderSystem::setViewport(value);
#ifdef _WINP8
		value = WinRT::rotateViewport(value);
#endif
		// this is needed on WinRT because of a graphics driver bug on Windows RT and on WinP8 because of a completely different graphics driver bug on Windows Phone 8
		static int w = 0;
		static int h = 0;
		if (w == 0 || h == 0)
		{
			gvec2 resolution = april::getSystemInfo().displayResolution;
			w = hround(resolution.x);
			h = hround(resolution.y);
			CHECK_SWAP(w, h);
		}
		if (value.x < 0.0f)
		{
			value.w += value.x;
			value.x = 0.0f;
		}
		if (value.y < 0.0f)
		{
			value.h += value.y;
			value.y = 0.0f;
		}
		value.w = hclamp(value.w, 0.0f, hmax(w - value.x, 0.0f));
		value.h = hclamp(value.h, 0.0f, hmax(h - value.y, 0.0f));
		if (value.w > 0.0f && value.h > 0.0f)
		{
			value.x = hclamp(value.x, 0.0f, (float)w);
			value.y = hclamp(value.y, 0.0f, (float)h);
		}
		else
		{
			value.set((float)w, (float)h, 0.0f, 0.0f);
		}
		// setting the system viewport
		D3D11_VIEWPORT viewport;
		viewport.MinDepth = D3D11_MIN_DEPTH;
		viewport.MaxDepth = D3D11_MAX_DEPTH;
		// these double-casts are to ensure consistent behavior among rendering systems
		viewport.TopLeftX = (float)(int)value.x;
		viewport.TopLeftY = (float)(int)value.y;
		viewport.Width = (float)(int)value.w;
		viewport.Height = (float)(int)value.h;
		this->d3dDeviceContext->RSSetViewports(1, &viewport);
	}

	void DirectX11_RenderSystem::setTextureBlendMode(BlendMode textureBlendMode)
	{
		switch (textureBlendMode)
		{
		case DEFAULT:
		case ALPHA_BLEND:
		case ADD:
		case SUBTRACT:
		case OVERWRITE:
			this->activeTextureBlendMode = textureBlendMode;
			break;
		default:
			hlog::warn(april::logTag, "Trying to set unsupported texture blend mode!");
			break;
		}
	}

	void DirectX11_RenderSystem::setTextureColorMode(ColorMode textureColorMode, unsigned char alpha)
	{
		this->activeTextureColorModeAlpha = 255;
		switch (textureColorMode)
		{
		case LERP: // LERP also needs alpha
			this->activeTextureColorModeAlpha = alpha;
		case NORMAL:
		case MULTIPLY:
		case ALPHA_MAP:
			this->activeTextureColorMode = textureColorMode;
			break;
		default:
			hlog::warn(april::logTag, "Trying to set unsupported texture color mode!");
			break;
		}
	}

	void DirectX11_RenderSystem::setTextureFilter(Texture::Filter textureFilter)
	{
		switch (textureFilter)
		{
		case Texture::FILTER_LINEAR:
		case Texture::FILTER_NEAREST:
			this->textureFilter = textureFilter;
			break;
		default:
			hlog::warn(april::logTag, "Trying to set unsupported texture filter!");
			break;
		}
	}

	void DirectX11_RenderSystem::setTextureAddressMode(Texture::AddressMode textureAddressMode)
	{
		switch (textureAddressMode)
		{
		case Texture::ADDRESS_WRAP:
		case Texture::ADDRESS_CLAMP:
			this->textureAddressMode = textureAddressMode;
			break;
		default:
			hlog::warn(april::logTag, "Trying to set unsupported texture address mode!");
			break;
		}
	}

	void DirectX11_RenderSystem::setTexture(Texture* texture)
	{
		this->activeTexture = (DirectX11_Texture*)texture;
		if (this->activeTexture != NULL)
		{
			Texture::Filter filter = this->activeTexture->getFilter();
			if (this->textureFilter != filter)
			{
				this->setTextureFilter(filter);
			}
			Texture::AddressMode addressMode = this->activeTexture->getAddressMode();
			if (this->textureAddressMode != addressMode)
			{
				this->setTextureAddressMode(addressMode);
			}
			this->activeTexture->load();
		}
	}

	Texture* DirectX11_RenderSystem::getRenderTarget()
	{
		return this->renderTarget;
	}
	
	void DirectX11_RenderSystem::setRenderTarget(Texture* source)
	{
		// TODO - test, this code is experimental
		DirectX11_Texture* texture = (DirectX11_Texture*)source;
		if (texture == NULL)
		{
			// has to use GetAddressOf(), because the parameter is a pointer to an array of render target views
			this->d3dDeviceContext->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), NULL);
		}
		else if (texture->d3dRenderTargetView != nullptr)
		{
			// has to use GetAddressOf(), because the parameter is a pointer to an array of render target views
			this->d3dDeviceContext->OMSetRenderTargets(1, texture->d3dRenderTargetView.GetAddressOf(), NULL);
		}
		else
		{
			hlog::error(april::logTag, "Texture not created as rendertarget: " + texture->_getInternalName());
			return;
		}
		this->renderTarget = texture;
		this->matrixDirty = true;
	}
	
	void DirectX11_RenderSystem::setPixelShader(PixelShader* pixelShader)
	{
		this->activePixelShader = (DirectX11_PixelShader*)pixelShader;
	}

	void DirectX11_RenderSystem::setVertexShader(VertexShader* vertexShader)
	{
		this->activeVertexShader = (DirectX11_VertexShader*)vertexShader;
	}

	void DirectX11_RenderSystem::_updatePixelShader(bool useTexture)
	{
		DirectX11_PixelShader* pixelShader = this->activePixelShader;
		if (pixelShader == NULL)
		{
			switch (this->activeTextureColorMode)
			{
			case NORMAL:
			case MULTIPLY:
				pixelShader = (useTexture ? this->pixelShaderTexturedMultiply : this->pixelShaderMultiply);
				break;
			case ALPHA_MAP:
				pixelShader = (useTexture ? this->pixelShaderTexturedAlphaMap : this->pixelShaderAlphaMap);
				break;
			case LERP:
				pixelShader = (useTexture ? this->pixelShaderTexturedLerp : this->pixelShaderLerp);
				break;
			}
		}
		if (this->_currentPixelShader != pixelShader)
		{
			this->_currentPixelShader = pixelShader;
			this->d3dDeviceContext->PSSetShader(this->_currentPixelShader->dx11Shader.Get(), NULL, 0);
		}
	}

	void DirectX11_RenderSystem::_updateVertexShader()
	{
		DirectX11_VertexShader* vertexShader = this->activeVertexShader;
		if (vertexShader == NULL)
		{
			vertexShader = this->vertexShaderDefault;
		}
		if (this->_currentVertexShader != vertexShader)
		{
			this->_currentVertexShader = vertexShader;
			this->d3dDeviceContext->VSSetShader(this->_currentVertexShader->dx11Shader.Get(), NULL, 0);
		}
	}

	Texture* DirectX11_RenderSystem::_createTexture(chstr filename)
	{
		return new DirectX11_Texture(filename);
	}

	Texture* DirectX11_RenderSystem::_createTexture(int w, int h, unsigned char* rgba)
	{
		return new DirectX11_Texture(w, h, rgba);
	}
	
	Texture* DirectX11_RenderSystem::_createTexture(int w, int h, Texture::Format format, Texture::Type type, Color color)
	{
		return new DirectX11_Texture(w, h, format, type, color);
	}
	
	PixelShader* DirectX11_RenderSystem::createPixelShader()
	{
		return new DirectX11_PixelShader();
	}

	PixelShader* DirectX11_RenderSystem::createPixelShader(chstr filename)
	{
		return new DirectX11_PixelShader(filename);
	}

	VertexShader* DirectX11_RenderSystem::createVertexShader()
	{
		return new DirectX11_VertexShader();
	}

	VertexShader* DirectX11_RenderSystem::createVertexShader(chstr filename)
	{
		return new DirectX11_VertexShader(filename);
	}

	void DirectX11_RenderSystem::clear(bool useColor, bool depth)
	{
		static const float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
		// TODO - should use current renderTargetView, not global one
		this->d3dDeviceContext->ClearRenderTargetView(this->renderTargetView.Get(), clearColor);
	}
	
	void DirectX11_RenderSystem::clear(bool depth, grect rect, Color color)
	{
		if (rect.w > 0.0f && rect.h > 0.0f)
		{
			const float clearColor[4] = {color.b_f(), color.g_f(), color.r_f(), color.a_f()};
			D3D11_RECT area;
			area.left = (int)rect.x;
			area.top = (int)rect.y;
			area.right = (int)(rect.x + rect.w);
			area.bottom = (int)(rect.y + rect.h);
			// TODO - should use current renderTargetView, not global one
			this->d3dDeviceContext->ClearView(this->renderTargetView.Get(), clearColor, &area, 1);
		}
	}

	void DirectX11_RenderSystem::_updateVertexBuffer(int nVertices, void* data)
	{
		static unsigned int size;
		static unsigned int vertexSize = sizeof(ColoredTexturedVertex);
		size = (unsigned int)(vertexSize * nVertices);
		if (size > this->vertexBufferDesc.ByteWidth)
		{
			this->vertexBuffer = nullptr;
			this->vertexBufferData.pSysMem = data;
			this->vertexBufferDesc.ByteWidth = size;
			this->vertexBufferDesc.StructureByteStride = vertexSize;
			this->d3dDevice->CreateBuffer(&this->vertexBufferDesc, &this->vertexBufferData, &this->vertexBuffer);
			this->_currentVertexBuffer = NULL;
		}
		else
		{
			this->d3dDeviceContext->Map(this->vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &this->mappedSubResource);
			memcpy(this->mappedSubResource.pData, data, size);
			this->d3dDeviceContext->Unmap(this->vertexBuffer.Get(), 0);
		}
		if (this->_currentVertexBuffer != this->vertexBuffer.GetAddressOf())
		{
			static unsigned int stride = vertexSize;
			static unsigned int offset = 0;
			this->d3dDeviceContext->IASetVertexBuffers(0, 1, this->vertexBuffer.GetAddressOf(), &stride, &offset);
		}
	}
	
	void DirectX11_RenderSystem::_updateConstantBuffer()
	{
		static float lerpAlpha = 1.0f;
		lerpAlpha = this->activeTextureColorModeAlpha / 255.0f;
		bool dirty = this->matrixDirty;
		if (!dirty)
		{
			dirty = (this->constantBufferData.lerpAlpha.w != lerpAlpha);
		}
		else // actually "if (this->matrixDirty)"
		{
			this->matrixDirty = false;
			this->constantBufferData.matrix = (this->projectionMatrix * this->modelviewMatrix).transposed();
#ifdef _WINP8 // hahaha, Windows Phone 8 is unable to rotate the display by itself!
			int angle = WinRT::getScreenRotation();
			if (angle != 0)
			{
				this->constantBufferData.matrix.rotateZ((float)angle);
			}
#endif
		}
		if (dirty)
		{
			this->constantBufferData.lerpAlpha.set(lerpAlpha, lerpAlpha, lerpAlpha, lerpAlpha);
			this->d3dDeviceContext->Map(this->constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &this->mappedSubResource);
			memcpy(this->mappedSubResource.pData, &this->constantBufferData, sizeof(ConstantBuffer));
			this->d3dDeviceContext->Unmap(this->constantBuffer.Get(), 0);
		}
	}

	void DirectX11_RenderSystem::_updateBlendMode()
	{
		static float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
		if (this->_currentTextureBlendMode != this->activeTextureBlendMode)
		{
			this->_currentTextureBlendMode = this->activeTextureBlendMode;
			switch (this->_currentTextureBlendMode)
			{
			case DEFAULT:
			case ALPHA_BLEND:
				this->d3dDeviceContext->OMSetBlendState(this->blendStateAlpha.Get(), blendFactor, 0xFFFFFFFF);
				break;
			case ADD:
				this->d3dDeviceContext->OMSetBlendState(this->blendStateAdd.Get(), blendFactor, 0xFFFFFFFF);
				break;
			case SUBTRACT:
				this->d3dDeviceContext->OMSetBlendState(this->blendStateSubtract.Get(), blendFactor, 0xFFFFFFFF);
				break;
			case OVERWRITE:
				this->d3dDeviceContext->OMSetBlendState(this->blendStateOverwrite.Get(), blendFactor, 0xFFFFFFFF);
				break;
			}
		}
	}

	void DirectX11_RenderSystem::_updateTexture(bool use)
	{
		if (!use)
		{
			return;
		}
		if (this->_currentTexture != this->activeTexture)
		{
			this->_currentTexture = this->activeTexture;
			if (this->_currentTexture != NULL)
			{
				this->d3dDeviceContext->PSSetShaderResources(0, 1, this->_currentTexture->d3dView.GetAddressOf());
			}
		}
		if (this->_currentTextureFilter != this->textureFilter ||
			this->_currentTextureAddressMode != this->textureAddressMode)
		{
			this->_currentTextureFilter = this->textureFilter;
			this->_currentTextureAddressMode = this->textureAddressMode;
			if (this->_currentTextureFilter == Texture::FILTER_LINEAR &&
				this->_currentTextureAddressMode == Texture::ADDRESS_WRAP)
			{
				this->d3dDeviceContext->PSSetSamplers(0, 1, this->samplerLinearWrap.GetAddressOf());
			}
			else if (this->_currentTextureFilter == Texture::FILTER_LINEAR &&
				this->_currentTextureAddressMode == Texture::ADDRESS_CLAMP)
			{
				this->d3dDeviceContext->PSSetSamplers(0, 1, this->samplerLinearClamp.GetAddressOf());
			}
			else if (this->_currentTextureFilter == Texture::FILTER_NEAREST &&
				this->_currentTextureAddressMode == Texture::ADDRESS_WRAP)
			{
				this->d3dDeviceContext->PSSetSamplers(0, 1, this->samplerNearestWrap.GetAddressOf());
			}
			else if (this->_currentTextureFilter == Texture::FILTER_NEAREST &&
				this->_currentTextureAddressMode == Texture::ADDRESS_CLAMP)
			{
				this->d3dDeviceContext->PSSetSamplers(0, 1, this->samplerNearestClamp.GetAddressOf());
			}
		}
	}
	
	void DirectX11_RenderSystem::_setRenderOp(RenderOp renderOp)
	{
		if (this->_currentRenderOp != renderOp)
		{
			this->_currentRenderOp = renderOp;
			this->d3dDeviceContext->IASetPrimitiveTopology(dx11_render_ops[this->_currentRenderOp]);
		}
	}

	void DirectX11_RenderSystem::render(RenderOp renderOp, PlainVertex* v, int nVertices)
	{
		this->render(renderOp, v, nVertices, april::Color::White);
	}

	void DirectX11_RenderSystem::render(RenderOp renderOp, PlainVertex* v, int nVertices, Color color)
	{
		ColoredTexturedVertex* ctv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_ctv : new ColoredTexturedVertex[nVertices];
		unsigned int c = (unsigned int)color;
		c = UINT_RGBA_TO_ABGR(c);
		for_iter (i, 0, nVertices)
		{
			ctv[i].x = v[i].x;
			ctv[i].y = v[i].y;
			ctv[i].z = v[i].z;
			ctv[i].color = c;
		}
		this->_setRenderOp(renderOp);
		this->_updateVertexBuffer(nVertices, ctv);
		this->_updateVertexShader();
		this->_updatePixelShader(false);
		this->_updateConstantBuffer();
		this->_updateBlendMode();
		this->_updateTexture(false);
		this->d3dDeviceContext->Draw(nVertices, 0);
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete [] ctv;
		}
	}

	void DirectX11_RenderSystem::render(RenderOp renderOp, TexturedVertex* v, int nVertices)
	{
		this->render(renderOp, v, nVertices, april::Color::White);
	}

	void DirectX11_RenderSystem::render(RenderOp renderOp, TexturedVertex* v, int nVertices, Color color)
	{
		ColoredTexturedVertex* ctv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_ctv : new ColoredTexturedVertex[nVertices];
		unsigned int c = (unsigned int)color;
		c = UINT_RGBA_TO_ABGR(c);
		for_iter (i, 0, nVertices)
		{
			ctv[i].x = v[i].x;
			ctv[i].y = v[i].y;
			ctv[i].z = v[i].z;
			ctv[i].u = v[i].u;
			ctv[i].v = v[i].v;
			ctv[i].color = c;
		}
		this->_setRenderOp(renderOp);
		this->_updateVertexBuffer(nVertices, ctv);
		this->_updateVertexShader();
		this->_updatePixelShader(true);
		this->_updateConstantBuffer();
		this->_updateBlendMode();
		this->_updateTexture(true);
		this->d3dDeviceContext->Draw(nVertices, 0);
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete [] ctv;
		}
	}

	void DirectX11_RenderSystem::render(RenderOp renderOp, ColoredVertex* v, int nVertices)
	{
		ColoredTexturedVertex* ctv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_ctv : new ColoredTexturedVertex[nVertices];
		for_iter (i, 0, nVertices)
		{
			ctv[i].x = v[i].x;
			ctv[i].y = v[i].y;
			ctv[i].z = v[i].z;
			ctv[i].color = UINT_RGBA_TO_ABGR(v[i].color);
		}
		this->_setRenderOp(renderOp);
		this->_updateVertexBuffer(nVertices, ctv);
		this->_updateVertexShader();
		this->_updatePixelShader(false);
		this->_updateConstantBuffer();
		this->_updateBlendMode();
		this->_updateTexture(false);
		this->d3dDeviceContext->Draw(nVertices, 0);
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete [] ctv;
		}
	}

	void DirectX11_RenderSystem::render(RenderOp renderOp, ColoredTexturedVertex* v, int nVertices)
	{
		ColoredTexturedVertex* ctv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_ctv : new ColoredTexturedVertex[nVertices];
		memcpy(ctv, v, sizeof(ColoredTexturedVertex) * nVertices);
		for_iter (i, 0, nVertices)
		{
			ctv[i].color = UINT_RGBA_TO_ABGR(v[i].color);
		}
		this->_setRenderOp(renderOp);
		this->_updateVertexBuffer(nVertices, ctv);
		this->_updateVertexShader();
		this->_updatePixelShader(true);
		this->_updateConstantBuffer();
		this->_updateBlendMode();
		this->_updateTexture(true);
		this->d3dDeviceContext->Draw(nVertices, 0);
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete [] ctv;
		}
	}

	void DirectX11_RenderSystem::_setModelviewMatrix(const gmat4& matrix)
	{
		this->matrixDirty = true;
	}

	void DirectX11_RenderSystem::_setProjectionMatrix(const gmat4& matrix)
	{
		this->matrixDirty = true;
	}

	Image* DirectX11_RenderSystem::takeScreenshot(int bpp)
	{
		// TODO - if possible
		hlog::warn(april::logTag, "DirectX11_RenderSystem::takeScreenshot() not implemented!");
		return NULL;
	}
	
	void DirectX11_RenderSystem::presentFrame()
	{
		this->swapChain->Present(2, 0);
		// has to use GetAddressOf(), because the parameter is a pointer to an array of render target views
		this->d3dDeviceContext->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), NULL);
	}

}

#endif
