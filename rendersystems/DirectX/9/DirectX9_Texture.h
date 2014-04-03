/// @file
/// @author  Kresimir Spes
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
/// Defines a DirectX9 specific texture.

#ifdef _DIRECTX9
#ifndef APRIL_DIRECTX9_TEXTURE_H
#define APRIL_DIRECTX9_TEXTURE_H

#include "DirectX_Texture.h"

struct IDirect3DTexture9;
struct IDirect3DSurface9;

namespace april
{
	class DirectX9_RenderSystem;

	class DirectX9_Texture : public DirectX_Texture
	{
	public:
		friend class DirectX9_RenderSystem;

		DirectX9_Texture(bool fromResource);
		~DirectX9_Texture();
		void unload();
		
		bool isLoaded();
		
	protected:
		IDirect3DSurface9* d3dSurface;
		IDirect3DTexture9* d3dTexture;
		D3DFORMAT d3dFormat;
		D3DPOOL d3dPool;
		DWORD d3dUsage;
		bool renderTarget;

		bool _createInternalTexture(unsigned char* data, int size, Type type);
		void _assignFormat();

		IDirect3DSurface9* _getSurface();

		Lock _tryLockSystem(int x, int y, int w, int h);
		bool _unlockSystem(Lock& lock, bool update);
		bool _uploadToGpu(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);

	};

}

#endif
#endif
