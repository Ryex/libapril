/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.31
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _DIRECTX9
#include <d3d9.h>

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Color.h"
#include "DirectX9_RenderSystem.h"
#include "DirectX9_Texture.h"
#include "Image.h"

#define APRIL_D3D_DEVICE (((DirectX9_RenderSystem*)april::rendersys)->d3dDevice)

namespace april
{
	DirectX9_Texture::DirectX9_Texture(bool fromResource) : DirectX_Texture(fromResource), d3dTexture(NULL), d3dSurface(NULL), d3dFormat(D3DFMT_UNKNOWN), renderTarget(false)
	{
	}

	DirectX9_Texture::~DirectX9_Texture()
	{
		this->unload();
	}

	bool DirectX9_Texture::_createInternalTexture(unsigned char* data, int size, Type type)
	{
		this->d3dPool = D3DPOOL_DEFAULT;
		this->d3dUsage = 0;
		// some GPUs seem to have problems creating off-screen A8 surfaces when D3DPOOL_DEFAULT is used
		if (!((DirectX9_RenderSystem*)april::rendersys)->_supportsA8Surface && this->d3dFormat == D3DFMT_A8)
		{
			this->d3dPool = D3DPOOL_MANAGED;
		}
		else if (this->type != TYPE_IMMUTABLE)
		{
			this->d3dUsage = D3DUSAGE_DYNAMIC;
		}
		// TODOaa - change pool to save memory
		/*
		if (type == TYPE_RENDER_TARGET)
		{
			this->d3dUsage = D3DUSAGE_RENDERTARGET;
			this->renderTarget = true;
		}
		*/
		HRESULT hr = APRIL_D3D_DEVICE->CreateTexture(this->width, this->height, 1, this->d3dUsage, this->d3dFormat, this->d3dPool, &this->d3dTexture, NULL);
		if (FAILED(hr))
		{
			hlog::error(april::logTag, "Failed to create DX9 texture!");
			return false;
		}
		return true;
	}
	
	void DirectX9_Texture::_assignFormat()
	{
		Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
		switch (nativeFormat)
		{
		case Image::FORMAT_BGRA:
			this->d3dFormat = D3DFMT_A8R8G8B8;
			break;
		case Image::FORMAT_BGRX:
			this->d3dFormat = D3DFMT_X8R8G8B8;
			break;
		case Image::FORMAT_ALPHA:
			this->d3dFormat = D3DFMT_A8; // lots of GPUs don't support A8 (even modern ones) so we use L8
			break;
		case Image::FORMAT_GRAYSCALE:
			this->d3dFormat = D3DFMT_L8;
			break;
		case Image::FORMAT_PALETTE: // TODOaa - needs changing, ARGB shouldn't be here
			this->d3dFormat = D3DFMT_A8R8G8B8;
			break;
		}
	}

	void DirectX9_Texture::unload()
	{
		if (this->d3dTexture != NULL)
		{
			hlog::write(april::logTag, "Unloading DX9 texture: " + this->_getInternalName());
			if (this->d3dSurface != NULL)
			{
				this->d3dSurface->Release();
				this->d3dSurface = NULL;
			}
			this->d3dTexture->Release();
			this->d3dTexture = NULL;
		}
	}

	bool DirectX9_Texture::isLoaded()
	{
		return (this->d3dTexture != NULL);
	}

	IDirect3DSurface9* DirectX9_Texture::_getSurface()
	{
		if (this->d3dSurface == NULL)
		{
			this->d3dTexture->GetSurfaceLevel(0, &this->d3dSurface);
		}
		return this->d3dSurface;
	}

	Texture::Lock DirectX9_Texture::_tryLockSystem(int x, int y, int w, int h)
	{
		Lock lock;
		D3DLOCKED_RECT lockRect;
		HRESULT hr;
		Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
		int nativeBpp = Image::getFormatBpp(nativeFormat);
		if (this->d3dPool == D3DPOOL_MANAGED)
		{
			RECT rect;
			rect.left = x;
			rect.top = y;
			rect.right = x + w;
			rect.bottom = y + h;
			hr = this->d3dTexture->LockRect(0, &lockRect, &rect, D3DLOCK_DISCARD);
			if (!FAILED(hr))
			{
				lock.systemBuffer = this->d3dTexture;
				lock.activateLock(0, 0, w, h, x, y, (unsigned char*)lockRect.pBits, lockRect.Pitch / nativeBpp, h, nativeFormat);
			}
			return lock;
		}
		IDirect3DSurface9* surface = NULL;
		hr = APRIL_D3D_DEVICE->CreateOffscreenPlainSurface(w, h, this->d3dFormat, D3DPOOL_SYSTEMMEM, &surface, NULL);
		if (FAILED(hr))
		{
			return lock;
		}
		hr = surface->LockRect(&lockRect, NULL, D3DLOCK_DISCARD);
		if (FAILED(hr))
		{
			surface->Release();
			return lock;
		}
		lock.systemBuffer = surface;
		// a D3DLOCKED_RECT always has a "pitch" that is a multiple of 4
		lock.activateLock(0, 0, w, h, x, y, (unsigned char*)lockRect.pBits, lockRect.Pitch / nativeBpp, h, nativeFormat);
		return lock;
		// TODOaa - render target locking
		/*
		Lock lock;
		D3DLOCKED_RECT lockRect;
		HRESULT hr = this->d3dTexture->LockRect(0, &lockRect, NULL, D3DLOCK_DISCARD);
		if (!FAILED(hr))
		{
			lock.buffer = this->_getSurface();
			lock.activateLock((unsigned char*)lockRect.pBits);
			return lock;
		}
		// could be a render target
		hr = APRIL_D3D_DEVICE->CreateOffscreenPlainSurface(this->width, this->height, this->d3dFormat, D3DPOOL_SYSTEMMEM, &lock.buffer, NULL);
		if (FAILED(hr))
		{
			hlog::error(april::logTag, "Failed to get pixel data, CreateOffscreenPlainSurface() call failed!");
			return lock;
		}
		hr = APRIL_D3D_DEVICE->GetRenderTargetData(this->_getSurface(), lock.buffer);
		if (FAILED(hr))
		{
			hlog::error(april::logTag, "Failed to get pixel data, GetRenderTargetData() call failed!");
			return lock;
		}
		hr = lock.buffer->LockRect(&lockRect, NULL, D3DLOCK_DISCARD);
		if (FAILED(hr))
		{
			hlog::error(april::logTag, "Failed to get pixel data, surface lock failed!");
			return lock;
		}
		lock.activateRenderTarget((unsigned char*)lockRect.pBits);
		return lock;
		*/
	}

	bool DirectX9_Texture::_unlockSystem(Lock& lock, bool update)
	{
		if (lock.systemBuffer == NULL)
		{
			return false;
		}
		if (this->d3dPool == D3DPOOL_MANAGED)
		{
			IDirect3DTexture9* texture = (IDirect3DTexture9*)lock.systemBuffer;
			if (lock.locked)
			{
				texture->UnlockRect(0);
			}
		}
		else
		{
			IDirect3DSurface9* surface = (IDirect3DSurface9*)lock.systemBuffer;
			if (update)
			{
				if (lock.locked)
				{
					surface->UnlockRect();
					RECT rect;
					rect.left = lock.x;
					rect.top = lock.y;
					rect.right = lock.x + lock.w;
					rect.bottom = lock.y + lock.h;
					POINT dest;
					dest.x = lock.dx;
					dest.y = lock.dy;
					APRIL_D3D_DEVICE->UpdateSurface(surface, &rect, this->_getSurface(), &dest);
				}
				else if (lock.renderTarget)
				{
					// TODOaa - implement
					/*
					surface->UnlockRect();
					if (update)
					{
						APRIL_D3D_DEVICE->UpdateSurface(lock.buffer, NULL, this->_getSurface(), NULL);
					}
					*/
				}
			}
			surface->Release();
		}
		return true;
	}

	bool DirectX9_Texture::_uploadToGpu(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		this->load();
		Lock lock = this->_tryLockSystem(dx, dy, sw, sh);
		if (lock.failed)
		{
			return false;
		}
		bool result = Image::write(sx, sy, sw, sh, lock.x, lock.y, srcData, srcWidth, srcHeight, srcFormat, lock.data, lock.dataWidth, lock.dataHeight, lock.format);
		this->_unlockSystem(lock, true);
		return result;
	}

}
#endif
