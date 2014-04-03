/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.33
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _DIRECTX9
#include <d3d9.h>
#include <d3d9types.h>
#include <stdio.h>

#include <gtypes/Vector2.h>
#include <hltypes/exception.h>
#include <hltypes/hlog.h>
#include <hltypes/hplatform.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "DirectX9_PixelShader.h"
#include "DirectX9_RenderSystem.h"
#include "DirectX9_Texture.h"
#include "DirectX9_VertexShader.h"
#include "Image.h"
#include "Keys.h"
#include "Platform.h"
#include "RenderState.h"
#include "Timer.h"
#include "Win32_Window.h"

#define VERTICES_BUFFER_COUNT 65536
#define APRIL_DX9_CHILD L"AprilDX9Child"

#define PLAIN_FVF (D3DFVF_XYZ)
#define COLOR_FVF (D3DFVF_XYZ | D3DFVF_DIFFUSE)
#define TEX_FVF (D3DFVF_XYZ | D3DFVF_TEX1)
#define TEX_COLOR_FVF (D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_DIFFUSE)
#define TEX_COLOR_TONE_FVF (D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_SPECULAR)

#define UINT_RGBA_TO_ARGB(c) ((((c) >> 8) & 0xFFFFFF) | (((c) & 0xFF) << 24))

#define IS_WINDOW_RESIZABLE (april::window->getName() == "Win32" && april::window->getOptions().resizable)

namespace april
{
	static ColoredTexturedVertex static_ctv[VERTICES_BUFFER_COUNT];
	static ColoredVertex static_cv[VERTICES_BUFFER_COUNT];

	D3DPRIMITIVETYPE dx9_render_ops[]=
	{
		D3DPT_FORCE_DWORD,
		D3DPT_TRIANGLELIST,		// ROP_TRIANGLE_LIST
		D3DPT_TRIANGLESTRIP,	// ROP_TRIANGLE_STRIP
		D3DPT_TRIANGLEFAN,		// ROP_TRIANGLE_FAN
		D3DPT_LINELIST,			// ROP_LINE_LIST
		D3DPT_LINESTRIP,		// ROP_LINE_STRIP
		D3DPT_POINTLIST,		// ROP_POINT_LIST
	};

	DirectX9_RenderSystem::DirectX9_RenderSystem() : DirectX_RenderSystem(), textureCoordinatesEnabled(false),
		colorEnabled(false), d3d(NULL), d3dDevice(NULL), activeTexture(NULL), renderTarget(NULL), backBuffer(NULL)
	{
		this->name = APRIL_RS_DIRECTX9;
		this->state = new RenderState(); // TODOa
		this->_supportsA8Surface = false;
		this->childHWnd = 0;
	}

	DirectX9_RenderSystem::~DirectX9_RenderSystem()
	{
		this->destroy();
	}

	bool DirectX9_RenderSystem::create(RenderSystem::Options options)
	{
		if (!DirectX_RenderSystem::create(options))
		{
			return false;
		}
		this->textureCoordinatesEnabled = false;
		this->colorEnabled = false;
		this->renderTarget = NULL;
		this->backBuffer = NULL;
		this->activeTexture = NULL;
		this->childHWnd = 0;
		// Direct3D
		this->d3d = Direct3DCreate9(D3D_SDK_VERSION);
		if (this->d3d == NULL)
		{
			this->destroy();
			throw hl_exception("Unable to create Direct3D9 object!");
		}
		this->d3dpp = new _D3DPRESENT_PARAMETERS_();
		return true;
	}

	bool DirectX9_RenderSystem::destroy()
	{
		if (!DirectX_RenderSystem::destroy())
		{
			return false;
		}
		if (this->d3dpp != NULL)
		{
			delete this->d3dpp;
			this->d3dpp = NULL;
		}
		if (this->d3dDevice != NULL)
		{
			this->d3dDevice->Release();
			this->d3dDevice = NULL;
		}
		if (this->d3d != NULL)
		{
			this->d3d->Release();
			this->d3d = NULL;
		}
		if (this->childHWnd != 0)
		{
			DestroyWindow(this->childHWnd);
			UnregisterClassW(APRIL_DX9_CHILD, GetModuleHandle(0));
			this->childHWnd = 0;
		}
		return true;
	}

	void DirectX9_RenderSystem::reset()
	{
		RenderSystem::reset();
		this->d3dDevice->EndScene();
		foreach (Texture*, it, this->textures)
		{
			(*it)->unload();
		}
		this->backBuffer->Release();
		this->backBuffer = NULL;
		HRESULT hr;
		while (april::window->isRunning())
		{
			hlog::write(april::logTag, "Resetting device...");
			if (this->d3dpp->BackBufferWidth <= 0 || this->d3dpp->BackBufferHeight <= 0)
			{
				throw hl_exception(hsprintf("Backbuffer size is invalid: %d x %d", this->d3dpp->BackBufferWidth, this->d3dpp->BackBufferHeight));
			}
			hr = this->d3dDevice->Reset(this->d3dpp);
			if (!FAILED(hr))
			{
				break;
			}
			if (hr == D3DERR_DRIVERINTERNALERROR)
			{
				throw hl_exception("Unable to reset Direct3D device, Driver Internal Error!");
			}
			else if (hr == D3DERR_OUTOFVIDEOMEMORY)
			{
				throw hl_exception("Unable to reset Direct3D device, Out of Video Memory!");
			}
			else
			{
				hlog::error(april::logTag, "Failed to reset device!");
			}
		}
		this->d3dDevice->GetRenderTarget(0, &this->backBuffer); // update backbuffer pointer
		this->d3dDevice->BeginScene();
		this->_configureDevice();
		this->setViewport(this->viewport);
		this->setOrthoProjection(this->orthoProjection);
		this->_setModelviewMatrix(this->modelviewMatrix);
		this->_setProjectionMatrix(this->projectionMatrix);
		hlog::write(april::logTag, "Direct3D9 Device restored.");
		// this is used to display window content while resizing window
		april::window->performUpdate(0.0f);
	}

	void DirectX9_RenderSystem::assignWindow(Window* window)
	{
		HWND hWnd = (HWND)april::window->getBackendId();
		memset(this->d3dpp, 0, sizeof(*this->d3dpp));
		bool resizable = IS_WINDOW_RESIZABLE;
		this->d3dpp->Windowed = !april::window->isFullscreen();
		int w = april::window->getWidth();
		int h = april::window->getHeight();
		if (resizable)
		{
			SystemInfo info = april::getSystemInfo();
			w = (int)info.displayResolution.x;
			h = (int)info.displayResolution.y;
		}
		this->d3dpp->BackBufferWidth = w;
		this->d3dpp->BackBufferHeight = h;
		this->d3dpp->BackBufferFormat = D3DFMT_X8R8G8B8;
		this->d3dpp->PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		if (this->options.depthBuffer)
		{
			this->d3dpp->EnableAutoDepthStencil = TRUE;
			this->d3dpp->AutoDepthStencilFormat = D3DFMT_D16;
		}
		this->d3dpp->SwapEffect = D3DSWAPEFFECT_COPY; // COPY is being used here as otherwise some weird tearing manifests during rendering
		this->d3dpp->hDeviceWindow = hWnd;
		if (resizable)
		{
			WNDCLASSEXW wc;
			memset(&wc, 0, sizeof(WNDCLASSEX));
			wc.cbSize = sizeof(WNDCLASSEX);
			wc.lpfnWndProc = &Win32_Window::childProcessCallback;
			wc.hInstance = GetModuleHandle(0);
			wc.hCursor = GetCursor();
			wc.lpszClassName = APRIL_DX9_CHILD;
			wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
			RegisterClassExW(&wc);
			this->childHWnd = CreateWindowW(APRIL_DX9_CHILD, APRIL_DX9_CHILD, WS_CHILD, 0, 0, w, h, hWnd, NULL, wc.hInstance, NULL);
			if (!window->isFullscreen())
			{
				this->_tryAssignChildWindow();
			}
		}
		HRESULT hr = this->d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, this->d3dpp, &this->d3dDevice);
		if (FAILED(hr))
		{
			hr = this->d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, this->d3dpp, &this->d3dDevice);
			if (FAILED(hr))
			{
				throw hl_exception("Unable to create Direct3D Device!");
			}
		}
		if (!this->_supportsA8Surface)
		{
			IDirect3DSurface9* surface = NULL;
			HRESULT hr = this->d3dDevice->CreateOffscreenPlainSurface(16, 16, D3DFMT_A8, D3DPOOL_SYSTEMMEM, &surface, NULL);
			if (!FAILED(hr))
			{
				this->_supportsA8Surface = true;
				surface->Release();
			}
		}
		this->clear();
		this->presentFrame();
		// device config
		this->d3dDevice->GetRenderTarget(0, &this->backBuffer);
		this->d3dDevice->BeginScene();
		this->_configureDevice();
		this->clear();
		this->setViewport(grect(0.0f, 0.0f, april::window->getSize()));
		this->orthoProjection.setSize(april::window->getSize());
		this->renderTarget = NULL;
	}

	void DirectX9_RenderSystem::_tryAssignChildWindow()
	{
		this->d3dpp->hDeviceWindow = this->childHWnd;
		ShowWindow(this->childHWnd, SW_SHOW);
		UpdateWindow(this->childHWnd);
	}

	void DirectX9_RenderSystem::_tryUnassignChildWindow()
	{
		HWND hWnd = (HWND)april::window->getBackendId();
		this->d3dpp->hDeviceWindow = hWnd;
		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);
	}
	
	void DirectX9_RenderSystem::_configureDevice()
	{
		// calls on init and device reset
		this->d3dDevice->SetRenderState(D3DRS_LIGHTING, 0);
		this->d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		this->d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
		this->d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		this->d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		this->d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		// separate alpha blending to use proper alpha blending
		this->d3dDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, 1);
		this->d3dDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
		this->d3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
		this->d3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
		// vertex color blending
		this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		this->d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		// misc
		this->setTextureBlendMode(april::BM_DEFAULT);
		this->setTextureColorMode(april::CM_DEFAULT);
		this->textureFilter = Texture::FILTER_UNDEFINED;
		this->textureAddressMode = Texture::ADDRESS_UNDEFINED;
	}

	int DirectX9_RenderSystem::getMaxTextureSize()
	{
		if (this->d3dDevice == NULL)
		{
			return 0;
		}
		D3DCAPS9 caps;
		this->d3dDevice->GetDeviceCaps(&caps);
		return caps.MaxTextureWidth;
	}

	harray<RenderSystem::DisplayMode> DirectX9_RenderSystem::getSupportedDisplayModes()
	{
		if (this->supportedDisplayModes.size() == 0)
		{
			IDirect3D9* d3d = this->d3d;
			if (this->d3d == NULL)
			{
				d3d = Direct3DCreate9(D3D_SDK_VERSION);
				if (d3d == NULL)
				{
					throw hl_exception("Unable to create Direct3D9 object!");
				}
			}
			unsigned int modeCount = d3d->GetAdapterModeCount(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8);
			HRESULT hr;
			D3DDISPLAYMODE displayMode;
			for_itert (unsigned int, i, 0, modeCount)
			{
				memset(&displayMode, 0, sizeof(D3DDISPLAYMODE));
				hr = d3d->EnumAdapterModes(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8, i, &displayMode);
				if (!FAILED(hr)) 
				{
					this->supportedDisplayModes += RenderSystem::DisplayMode(displayMode.Width, displayMode.Height, displayMode.RefreshRate);
				}
			}
			if (this->d3d == NULL)
			{
				d3d->Release();
			}
		}
		return this->supportedDisplayModes;
	}

	void DirectX9_RenderSystem::setViewport(grect rect)
	{
		RenderSystem::setViewport(rect);
		D3DVIEWPORT9 viewport;
		viewport.MinZ = 0.0f;
		viewport.MaxZ = 1.0f;
		viewport.X = (int)rect.x;
		viewport.Y = (int)rect.y;
		viewport.Width = (int)rect.w;
		viewport.Height = (int)rect.h;
		this->d3dDevice->SetViewport(&viewport);
	}

	void DirectX9_RenderSystem::setTextureBlendMode(BlendMode textureBlendMode)
	{
		switch (textureBlendMode)
		{
		case BM_DEFAULT:
		case BM_ALPHA:
			this->d3dDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
			this->d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			break;
		case BM_ADD:
			this->d3dDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
			this->d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			break;
		case BM_SUBTRACT:
			this->d3dDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
			this->d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			break;
		case BM_OVERWRITE:
			this->d3dDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ZERO);
			this->d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
			break;
		default:
			hlog::warn(april::logTag, "Trying to set unsupported texture blend mode!");
			break;
		}
	}

	void DirectX9_RenderSystem::setTextureColorMode(ColorMode textureColorMode, float factor)
	{
		static unsigned char color = 0;
		color = (unsigned char)(factor * 255);
		switch (textureColorMode)
		{
		case CM_DEFAULT:
		case CM_MULTIPLY:
			this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			break;
		case CM_LERP:
			this->d3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(255, color, color, color));
			this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_LERP);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG0, D3DTA_TFACTOR);
			break;
		case CM_ALPHA_MAP:
			this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			break;
		default:
			hlog::warn(april::logTag, "Trying to set unsupported texture color mode!");
			break;
		}
	}

	void DirectX9_RenderSystem::setTextureFilter(Texture::Filter textureFilter)
	{
		this->textureFilter = textureFilter;
		switch (textureFilter)
		{
		case Texture::FILTER_LINEAR:
			this->d3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			this->d3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			break;
		case Texture::FILTER_NEAREST:
			this->d3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
			this->d3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
			break;
		default:
			hlog::warn(april::logTag, "Trying to set unsupported texture filter!");
			break;
		}
	}

	void DirectX9_RenderSystem::setTextureAddressMode(Texture::AddressMode textureAddressMode)
	{
		this->textureAddressMode = textureAddressMode;
		switch (textureAddressMode)
		{
		case Texture::ADDRESS_WRAP:
			this->d3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
			this->d3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
			break;
		case Texture::ADDRESS_CLAMP:
			this->d3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
			this->d3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
			break;
		default:
			hlog::warn(april::logTag, "Trying to set unsupported texture address mode!");
			break;
		}
	}

	void DirectX9_RenderSystem::setTexture(Texture* texture)
	{
		this->activeTexture = (DirectX9_Texture*)texture;
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
			this->d3dDevice->SetTexture(0, this->activeTexture->d3dTexture);
		}
		else
		{
			this->d3dDevice->SetTexture(0, NULL);
		}
	}

	Texture* DirectX9_RenderSystem::getRenderTarget()
	{
		return this->renderTarget;
	}
	
	void DirectX9_RenderSystem::setRenderTarget(Texture* source)
	{
		if (this->renderTarget != NULL)
		{
			this->d3dDevice->EndScene();
		}
		DirectX9_Texture* texture = (DirectX9_Texture*)source;
		if (texture == NULL)
		{
			this->d3dDevice->SetRenderTarget(0, this->backBuffer);
		}
		else
		{
			this->d3dDevice->SetRenderTarget(0, texture->_getSurface());
		}
		this->renderTarget = texture;
		if (this->renderTarget != NULL)
		{
			this->d3dDevice->BeginScene();
		}
	}
	
	void DirectX9_RenderSystem::setPixelShader(PixelShader* pixelShader)
	{
		DirectX9_PixelShader* shader = (DirectX9_PixelShader*)pixelShader;
		if (shader != NULL)
		{
			this->d3dDevice->SetPixelShader(shader->dx9Shader);
		}
		else
		{
			this->d3dDevice->SetPixelShader(NULL);
		}
	}

	void DirectX9_RenderSystem::setVertexShader(VertexShader* vertexShader)
	{
		DirectX9_VertexShader* shader = (DirectX9_VertexShader*)vertexShader;
		if (shader != NULL)
		{
			this->d3dDevice->SetVertexShader(shader->dx9Shader);
		}
		else
		{
			this->d3dDevice->SetVertexShader(NULL);
		}
	}

	void DirectX9_RenderSystem::_setResolution(int w, int h, bool fullscreen)
	{
		if (this->backBuffer == NULL)
		{
			return;
		}
		if (w <= 0 || h <= 0)
		{
			hlog::warnf(april::logTag, "Cannot set resolution to: %d x %d", w, h);
			return;
		}
		bool resizable = IS_WINDOW_RESIZABLE;
		bool oldFullscreen = (this->d3dpp->Windowed == FALSE);
		this->d3dpp->Windowed = !fullscreen;
		if (fullscreen != oldFullscreen || !resizable)
		{
			if (resizable)
			{
				if (!fullscreen)
				{
					this->_tryAssignChildWindow();
				}
				else
				{
					this->_tryUnassignChildWindow();
				}
				this->setViewport(grect(0.0f, 0.0f, (float)w, (float)h));
				SystemInfo info = april::getSystemInfo();
				w = (int)info.displayResolution.x;
				h = (int)info.displayResolution.y;
			}
			this->d3dpp->BackBufferWidth = w;
			this->d3dpp->BackBufferHeight = h;
			this->reset();
		}
		else
		{
			if (resizable)
			{
				this->_tryAssignChildWindow();
			}
			this->setViewport(grect(0.0f, 0.0f, (float)w, (float)h));
		}
	}

	Texture* DirectX9_RenderSystem::_createTexture(bool fromResource)
	{
		return new DirectX9_Texture(fromResource);
	}

	PixelShader* DirectX9_RenderSystem::createPixelShader()
	{
		return new DirectX9_PixelShader();
	}

	PixelShader* DirectX9_RenderSystem::createPixelShader(chstr filename)
	{
		return new DirectX9_PixelShader(filename);
	}

	VertexShader* DirectX9_RenderSystem::createVertexShader()
	{
		return new DirectX9_VertexShader();
	}

	VertexShader* DirectX9_RenderSystem::createVertexShader(chstr filename)
	{
		return new DirectX9_VertexShader(filename);
	}

	void DirectX9_RenderSystem::clear(bool useColor, bool depth)
	{
		DWORD flags = 0;
		if (useColor)
		{
			flags |= D3DCLEAR_TARGET;
		}
		if (depth && this->options.depthBuffer)
		{
			flags |= D3DCLEAR_ZBUFFER;
		}
		this->d3dDevice->Clear(0, NULL, flags, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
	}
	
	void DirectX9_RenderSystem::clear(bool depth, grect rect, Color color)
	{
		DWORD flags = 0;
		flags |= D3DCLEAR_TARGET;
		if (depth && this->options.depthBuffer)
		{
			flags |= D3DCLEAR_ZBUFFER;
		}
		D3DRECT area;
		area.x1 = (int)rect.x;
		area.y1 = (int)rect.y;
		area.x2 = (int)(rect.x + rect.w);
		area.y2 = (int)(rect.y + rect.h);
		this->d3dDevice->Clear(1, &area, flags, D3DCOLOR_ARGB((int)color.a, (int)color.r, (int)color.g, (int)color.b), 1.0f, 0);
	}
	
	void DirectX9_RenderSystem::render(RenderOperation renderOperation, PlainVertex* v, int nVertices)
	{
		if (this->activeTexture != NULL)
		{
			this->setTexture(NULL);
		}
		this->d3dDevice->SetFVF(PLAIN_FVF);
		this->d3dDevice->DrawPrimitiveUP(dx9_render_ops[renderOperation], this->_numPrimitives(renderOperation, nVertices), v, sizeof(PlainVertex));
	}

	void DirectX9_RenderSystem::render(RenderOperation renderOperation, PlainVertex* v, int nVertices, Color color)
	{
		if (this->activeTexture != NULL)
		{
			this->setTexture(NULL);
		}
		unsigned int colorDx9 = D3DCOLOR_ARGB((int)color.a, (int)color.r, (int)color.g, (int)color.b);
		ColoredVertex* cv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_cv : new ColoredVertex[nVertices];
		for_iter (i, 0, nVertices)
		{
			cv[i].x = v[i].x;
			cv[i].y = v[i].y;
			cv[i].z = v[i].z;
			cv[i].color = colorDx9;
		}
		this->d3dDevice->SetFVF(COLOR_FVF);
		this->d3dDevice->DrawPrimitiveUP(dx9_render_ops[renderOperation], this->_numPrimitives(renderOperation, nVertices), cv, sizeof(ColoredVertex));
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete [] cv;
		}
	}
	
	void DirectX9_RenderSystem::render(RenderOperation renderOperation, TexturedVertex* v, int nVertices)
	{
		this->d3dDevice->SetFVF(TEX_FVF);
		this->d3dDevice->DrawPrimitiveUP(dx9_render_ops[renderOperation], this->_numPrimitives(renderOperation, nVertices), v, sizeof(TexturedVertex));
	}

	void DirectX9_RenderSystem::render(RenderOperation renderOperation, TexturedVertex* v, int nVertices, Color color)
	{
		unsigned int colorDx9 = D3DCOLOR_ARGB((int)color.a, (int)color.r, (int)color.g, (int)color.b);
		ColoredTexturedVertex* ctv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_ctv : new ColoredTexturedVertex[nVertices];
		for_iter (i, 0, nVertices)
		{
			ctv[i].x = v[i].x;
			ctv[i].y = v[i].y;
			ctv[i].z = v[i].z;
			ctv[i].u = v[i].u;
			ctv[i].v = v[i].v;
			ctv[i].color = colorDx9;
		}
		this->d3dDevice->SetFVF(TEX_COLOR_FVF);
		this->d3dDevice->DrawPrimitiveUP(dx9_render_ops[renderOperation], this->_numPrimitives(renderOperation, nVertices), ctv, sizeof(ColoredTexturedVertex));
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete [] ctv;
		}
	}

	void DirectX9_RenderSystem::render(RenderOperation renderOperation, ColoredVertex* v, int nVertices)
	{
		if (this->activeTexture != NULL)
		{
			this->setTexture(NULL);
		}
		ColoredVertex* cv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_cv : new ColoredVertex[nVertices];
		memcpy(cv, v, sizeof(ColoredVertex) * nVertices);
		for_iter (i, 0, nVertices)
		{
			cv[i].color = UINT_RGBA_TO_ARGB(v[i].color);
		}
		this->d3dDevice->SetFVF(COLOR_FVF);
		this->d3dDevice->DrawPrimitiveUP(dx9_render_ops[renderOperation], this->_numPrimitives(renderOperation, nVertices), cv, sizeof(ColoredVertex));
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete [] cv;
		}
	}

	void DirectX9_RenderSystem::render(RenderOperation renderOperation, ColoredTexturedVertex* v, int nVertices)
	{
		ColoredTexturedVertex* ctv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_ctv : new ColoredTexturedVertex[nVertices];
		memcpy(ctv, v, sizeof(ColoredTexturedVertex) * nVertices);
		for_iter (i, 0, nVertices)
		{
			ctv[i].color = UINT_RGBA_TO_ARGB(v[i].color);
		}
		this->d3dDevice->SetFVF(TEX_COLOR_FVF);
		this->d3dDevice->DrawPrimitiveUP(dx9_render_ops[renderOperation], this->_numPrimitives(renderOperation, nVertices), ctv, sizeof(ColoredTexturedVertex));
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete [] ctv;
		}
	}

	void DirectX9_RenderSystem::_setModelviewMatrix(const gmat4& matrix)
	{
		this->d3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)matrix.data);
	}

	void DirectX9_RenderSystem::_setProjectionMatrix(const gmat4& matrix)
	{
		this->d3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)matrix.data);
	}

	Image::Format DirectX9_RenderSystem::getNativeTextureFormat(Image::Format format)
	{
		switch (format)
		{
		case Image::FORMAT_RGBA:
		case Image::FORMAT_ARGB:
		case Image::FORMAT_BGRA:
		case Image::FORMAT_ABGR:
			return Image::FORMAT_BGRA;
		case Image::FORMAT_RGBX:
		case Image::FORMAT_XRGB:
		case Image::FORMAT_BGRX:
		case Image::FORMAT_XBGR:
		case Image::FORMAT_RGB:
		case Image::FORMAT_BGR:
			return Image::FORMAT_BGRX;
		case Image::FORMAT_ALPHA:
			return Image::FORMAT_ALPHA;
		case Image::FORMAT_GRAYSCALE:
			return Image::FORMAT_GRAYSCALE;
		case Image::FORMAT_PALETTE:
			return Image::FORMAT_PALETTE;
		}
		return Image::FORMAT_INVALID;
	}

	Image* DirectX9_RenderSystem::takeScreenshot(Image::Format format)
	{
#ifdef _DEBUG
		hlog::write(april::logTag, "Taking screenshot...");
#endif
		D3DSURFACE_DESC desc;
		this->backBuffer->GetDesc(&desc);
		if (desc.Format != D3DFMT_X8R8G8B8)
		{
			hlog::error(april::logTag, "Failed to grab screenshot, backbuffer format not supported, expected X8R8G8B8, got: " + hstr(desc.Format));
			return NULL;
		}
		IDirect3DSurface9* buffer;
		HRESULT hr = this->d3dDevice->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &buffer, NULL);
		if (FAILED(hr))
		{
			hlog::error(april::logTag, "Failed to grab screenshot, CreateOffscreenPlainSurface() call failed.");
			return NULL;
		}
		hr = this->d3dDevice->GetRenderTargetData(this->backBuffer, buffer);
		if (FAILED(hr))
		{
			hlog::error(april::logTag, "Failed to grab screenshot, GetRenderTargetData() call failed.");
			buffer->Release();
			return NULL;
		}		
		D3DLOCKED_RECT rect;
		hr = buffer->LockRect(&rect, NULL, D3DLOCK_DONOTWAIT);
		if (FAILED(hr))
		{
			hlog::error(april::logTag, "Failed to grab screenshot, surface lock failed.");
			buffer->Release();
			return NULL;
		}
		unsigned char* data = NULL;
		Image* image = NULL;
		if (Image::convertToFormat(desc.Width, desc.Height, (unsigned char*)rect.pBits, Image::FORMAT_BGRX, &data, format, false))
		{
			image = Image::create(desc.Width, desc.Height, data, format);
			delete [] data;
		}
		buffer->UnlockRect();
		buffer->Release();
		return image;
	}
	
	void DirectX9_RenderSystem::presentFrame()
	{
		this->d3dDevice->EndScene();
		HRESULT hr = this->d3dDevice->Present(NULL, NULL, NULL, NULL);
		if (hr == D3DERR_DEVICELOST)
		{
			hlog::write(april::logTag, "Direct3D9 Device lost, attempting to restore...");
			foreach (Texture*, it, this->textures)
			{
				(*it)->unload();
			}
			this->backBuffer->Release();
			this->backBuffer = NULL;
			while (april::window->isRunning())
			{
				for_iter (i, 0, 10)
				{
					april::window->checkEvents();
					hthread::sleep(100.0f);
				}
				hr = this->d3dDevice->TestCooperativeLevel();
				if (!FAILED(hr))
				{
					break;
				}
				if (hr == D3DERR_DEVICENOTRESET)
				{
					hlog::write(april::logTag, "Resetting device...");
					hr = this->d3dDevice->Reset(this->d3dpp);
					if (!FAILED(hr))
					{
						break;
					}
					if (hr == D3DERR_DRIVERINTERNALERROR)
					{
						throw hl_exception("Unable to reset Direct3D device, Driver Internal Error!");
					}
					else if (hr == D3DERR_OUTOFVIDEOMEMORY)
					{
						throw hl_exception("Unable to reset Direct3D device, Out of Video Memory!");
					}
					else
					{
						hlog::error(april::logTag, "Failed to reset device!");
					}
				}
				else if (hr == D3DERR_DRIVERINTERNALERROR)
				{
					throw hl_exception("Unable to reset Direct3D device, Driver Internal Error!");
				}
			}
			this->_configureDevice();
			this->d3dDevice->GetRenderTarget(0, &this->backBuffer); // update backbuffer pointer
			this->d3dDevice->BeginScene();
			this->setViewport(this->viewport);
			this->setOrthoProjection(this->orthoProjection);
			this->_setModelviewMatrix(this->modelviewMatrix);
			this->_setProjectionMatrix(this->projectionMatrix);
			hlog::write(april::logTag, "Direct3D9 Device restored.");
		}
		else
		{
			if (hr == D3DERR_WASSTILLDRAWING)
			{
				for_iter (i, 0, 100)
				{
					hr = this->d3dDevice->Present(NULL, NULL, NULL, NULL);
					if (!FAILED(hr))
					{
						break;
					}
					hthread::sleep(1.0f);
				}
			}
			this->d3dDevice->BeginScene();
		}
	}

}

#endif
