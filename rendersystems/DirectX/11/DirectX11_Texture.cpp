/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _DIRECTX11
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "DirectX11_RenderSystem.h"
#include "DirectX11_Texture.h"
#include "Image.h"

#define APRIL_D3D_DEVICE (((DirectX11_RenderSystem*)april::rendersys)->d3dDevice)
#define APRIL_D3D_DEVICE_CONTEXT (((DirectX11_RenderSystem*)april::rendersys)->d3dDeviceContext)
#define _CREATE_RECT(name, x, y, w, h) \
	D3D11_RECT name; \
	name.left = x; \
	name.top = y; \
	name.right = x + w - 1; \
	name.bottom = y + h - 1;

namespace april
{
	// TODO - refactor
	extern harray<DirectX11_Texture*> gRenderTargets;

	DirectX11_Texture::DirectX11_Texture(chstr filename) : Texture()
	{
		this->filename = filename;
		this->format = FORMAT_ARGB;
		this->width = 0;
		this->height = 0;
		this->bpp = 4;
		this->renderTarget = false;
		this->d3dTexture = nullptr;
		this->d3dView = nullptr;
		this->d3dRenderTargetView = nullptr;
		this->manualData = NULL;
		hlog::write(april::logTag, "Creating DX11 texture: " + this->_getInternalName());
	}

	DirectX11_Texture::DirectX11_Texture(int w, int h, unsigned char* rgba) : Texture()
	{
		this->filename = "";
		this->format = FORMAT_ARGB;
		this->width = w;
		this->height = h;
		this->bpp = 4;
		this->renderTarget = false;
		this->d3dTexture = nullptr;
		this->d3dView = nullptr;
		this->d3dRenderTargetView = nullptr;
		hlog::write(april::logTag, "Creating user-defined DX11 texture.");
		this->manualData = new unsigned char[this->width * this->height * this->bpp];
		memcpy(this->manualData, rgba, this->width * this->height * this->bpp); // so alpha doesn't have to be copied in each iteration
		int offset;
		int i;
		int j;
		for_iterx (j, 0, this->height)
		{
			for_iterx (i, 0, this->width)
			{
				offset = (i + j * this->width) * this->bpp;
				this->manualData[offset + 2] = rgba[offset + 0];
				//this->manualData[offset + 1] = rgba[offset + 1]; // not necessary to be executed because of previous memcpy call
				this->manualData[offset + 0] = rgba[offset + 2];
			}
		}
		this->_createInternalTexture(this->manualData);
	}
	
	DirectX11_Texture::DirectX11_Texture(int w, int h, Texture::Format format, Texture::Type type, Color color) : Texture()
	{
		this->filename = "";
		this->format = format;
		this->width = w;
		this->height = h;
		this->renderTarget = false;
		this->d3dTexture = nullptr;
		this->d3dView = nullptr;
		this->d3dRenderTargetView = nullptr;
		hlog::writef(april::logTag, "Creating empty DX11 texture (%d x %d).", w, h);
		this->bpp = 4;
		if (type == TYPE_RENDER_TARGET)
		{
			this->renderTarget = true;
			gRenderTargets += this;
		}
		this->manualData = new unsigned char[this->width * this->height * this->bpp];
		memset(this->manualData, 0, this->width * this->height * this->bpp);
		this->_createInternalTexture(this->manualData);
		if (color != Color::Clear)
		{
			this->fillRect(0, 0, this->width, this->height, color);
		}
	}

	bool DirectX11_Texture::_createInternalTexture(unsigned char* data)
	{
		// texture
		D3D11_SUBRESOURCE_DATA textureSubresourceData = {0};
		textureSubresourceData.pSysMem = data;
		textureSubresourceData.SysMemPitch = this->width * this->bpp;
		textureSubresourceData.SysMemSlicePitch = 0;
		D3D11_TEXTURE2D_DESC textureDesc = {0};
		textureDesc.Width = this->width;
		textureDesc.Height = this->height;
		textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		if (this->manualData == NULL)
		{
			textureDesc.Usage = D3D11_USAGE_DEFAULT;
			textureDesc.CPUAccessFlags = 0;
		}
		else
		{
			textureDesc.Usage = D3D11_USAGE_DYNAMIC;
			textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		textureDesc.MiscFlags = 0;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		switch (this->format)
		{
		case FORMAT_ARGB:
			textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			this->bpp = 4;
			break;
		case FORMAT_RGB:
			textureDesc.Format = DXGI_FORMAT_B8G8R8X8_UNORM;
			this->bpp = 4;
			break;
		case FORMAT_ALPHA:
			textureDesc.Format = DXGI_FORMAT_R8_UNORM;
			this->bpp = 1;
			break;
		default:
			textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			this->bpp = 4;
			break;
		}
		textureSubresourceData.SysMemPitch = this->width * this->bpp;
		if (this->renderTarget)
		{
			textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		}
		HRESULT hr = APRIL_D3D_DEVICE->CreateTexture2D(&textureDesc, &textureSubresourceData, &this->d3dTexture);
		if (FAILED(hr))
		{
			hlog::error(april::logTag, "Failed to create DX11 texture!");
			return false;
		}
		if (this->renderTarget)
		{
			D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
			renderTargetViewDesc.Format = textureDesc.Format;
			renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			renderTargetViewDesc.Texture2D.MipSlice = 0;
			hr = APRIL_D3D_DEVICE->CreateRenderTargetView(this->d3dTexture.Get(),
				&renderTargetViewDesc, &this->d3dRenderTargetView);
			if (FAILED(hr))
			{
				hlog::error(april::logTag, "Failed to create render target view for texture with render-to-texture!");
				return false;
			}
		}
		// shader resource
		D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDesc;
		memset(&textureViewDesc, 0, sizeof(textureViewDesc));
		textureViewDesc.Format = textureDesc.Format;
		textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		textureViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
		textureViewDesc.Texture2D.MostDetailedMip = 0;
		hr = APRIL_D3D_DEVICE->CreateShaderResourceView(this->d3dTexture.Get(), &textureViewDesc, &this->d3dView);
		if (FAILED(hr))
		{
			hlog::error(april::logTag, "Failed to create DX11 texture view!");
			return false;
		}
		return true;
	}
	
	DirectX11_Texture::~DirectX11_Texture()
	{
		this->unload();
		if (this->renderTarget)
		{
			gRenderTargets -= this;
		}
	}
	
	bool DirectX11_Texture::load()
	{
		if (this->isLoaded())
		{
			return true;
		}
		hlog::write(april::logTag, "Loading DX11 texture: " + this->_getInternalName());
		Image* image = NULL;
		if (this->filename != "")
		{
			image = Image::load(this->filename);
			if (image == NULL)
			{
				hlog::error(april::logTag, "Failed to load texture: " + this->_getInternalName());
				return false;
			}
			this->width = image->w;
			this->height = image->h;
			this->bpp = image->bpp;
		}
		if (image == NULL)
		{
			hlog::error(april::logTag, "Image source does not exist!");
			return false;
		}
		switch (image->format)
		{
		case Image::FORMAT_RGBA:
		case Image::FORMAT_BGRA:
			this->format = FORMAT_ARGB;
			break;
		case Image::FORMAT_RGB:
		case Image::FORMAT_BGR:
			this->format = FORMAT_RGB;
			break;
		case Image::FORMAT_GRAYSCALE:
			this->format = FORMAT_ALPHA;
			break;
		case Image::FORMAT_PALETTE:
			this->format = FORMAT_ARGB; // TODO - should be changed
			break;
		default:
			this->format = FORMAT_ARGB;
			break;
		}
		if (image->format == Image::FORMAT_RGBA || image->format == Image::FORMAT_RGB)
		{
			this->manualData = new unsigned char[image->w * image->h * 4];
			memset(this->manualData, 255, image->w * image->h * 4);
			if (image->format == Image::FORMAT_RGBA)
			{
				image->copyPixels(this->manualData, Image::FORMAT_BGRA);
			}
			else
			{
				image->copyPixels(this->manualData, Image::FORMAT_BGR);
			}
			this->bpp = image->bpp = 4;
		}
		else
		{
			this->manualData = image->data;
			image->data = NULL;
		}
		bool result = this->_createInternalTexture(this->manualData);
		delete image;
		return result;
	}
	
	void DirectX11_Texture::unload()
	{
		if (this->d3dTexture != nullptr)
		{
			hlog::write(april::logTag, "Unloading DX11 texture: " + this->_getInternalName());
			this->d3dTexture = nullptr;
			this->d3dView = nullptr;
			this->d3dRenderTargetView = nullptr;
		}
		_HL_TRY_DELETE(this->manualData);
	}
	
	bool DirectX11_Texture::isLoaded()
	{
		return (this->d3dTexture != nullptr);
	}
	
	void DirectX11_Texture::clear()
	{
		memset(this->manualData, 0, this->getByteSize());
		this->_updateTexture();
	}
	
	Color DirectX11_Texture::getPixel(int x, int y)
	{
		Color color = Color::Clear;
		unsigned char* p = this->manualData + (y * this->width + x) * this->bpp;
		if (this->bpp == 4)
		{
			color.r = p[2];
			color.g = p[1];
			color.b = p[0];
			color.a = p[3];
		}
		else if (this->bpp == 3)
		{
			color.r = p[2];
			color.g = p[1];
			color.b = p[0];
			color.a = 255;
		}
		else if (this->bpp == 1)
		{
			color.r = 255;
			color.g = 255;
			color.b = 255;
			color.a = p[0];
		}
		else
		{
			hlog::error(april::logTag, "Unsupported format for getPixel()!");
		}
		return color;
	}
	
	void DirectX11_Texture::setPixel(int x, int y, Color color)
	{
		x = hclamp(x, 0, this->width - 1);
		y = hclamp(y, 0, this->height - 1);
		unsigned char* p = this->manualData + (y * this->width + x) * this->bpp;
		if (this->bpp == 4)
		{
			p[2] = color.r;
			p[1] = color.g;
			p[0] = color.b;
			p[3] = color.a;
		}
		else if (this->bpp == 3)
		{
			p[2] = color.r;
			p[1] = color.g;
			p[0] = color.b;
		}
		else if (this->bpp == 1)
		{
			p[0] = (color.r + color.g + color.b) / 3;
		}
		else
		{
			hlog::error(april::logTag, "Unsupported format for setPixel()!");
		}
		this->_updateTexture();
	}
	
	void DirectX11_Texture::fillRect(int x, int y, int w, int h, Color color)
	{
		x = hclamp(x, 0, this->width - 1);
		y = hclamp(y, 0, this->height - 1);
		w = hclamp(w, 1, this->width - x);
		h = hclamp(h, 1, this->height - y);
		if (w == 1 && h == 1)
		{
			this->setPixel(x, y, color);
			return;
		}
		unsigned char* p = this->manualData + (y * this->width + x) * this->bpp;
		int i;
		int j;
		int offset;
		if (this->bpp == 4)
		{
			for_iterx (j, 0, h)
			{
				for_iterx (i, 0, w)
				{
					offset = (j * this->width + i) * this->bpp;
					p[offset + 2] = color.r;
					p[offset + 1] = color.g;
					p[offset + 0] = color.b;
					p[offset + 3] = color.a;
				}
			}
		}
		else if (this->bpp == 3)
		{
			for_iterx (j, 0, h)
			{
				for_iterx (i, 0, w)
				{
					offset = (i + j * this->width) * this->bpp;
					p[offset + 2] = color.r;
					p[offset + 1] = color.g;
					p[offset + 0] = color.b;
				}
			}
		}
		else if (this->bpp == 1)
		{
			for_iterx (j, 0, h)
			{
				for_iterx (i, 0, w)
				{
					offset = (i + j * this->width) * this->bpp;
					p[offset] = (color.r + color.g + color.b) / 3;
				}
			}
		}
		else
		{
			hlog::error(april::logTag, "Unsupported format for setPixel()!");
		}
		this->_updateTexture();
	}
	
	void DirectX11_Texture::blit(int x, int y, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		DirectX11_Texture* source = (DirectX11_Texture*)texture;
		x = hclamp(x, 0, this->width - 1);
		y = hclamp(y, 0, this->height - 1);
		sx = hclamp(sx, 0, source->width - 1);
		sy = hclamp(sy, 0, source->height - 1);
		sw = hmin(sw, hmin(this->width - x, source->width - sx));
		sh = hmin(sh, hmin(this->height - y, source->height - sy));
		if (sw == 1 && sh == 1)
		{
			this->setPixel(x, y, source->getPixel(sx, sy));
			return;
		}
		unsigned char* s = source->manualData + (sy * source->width + sx) * source->bpp;
		this->blit(x, y, s, source->width, source->height, source->bpp, sx, sy, sw, sh, alpha);
	}
	
	void DirectX11_Texture::blit(int x, int y, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		x = hclamp(x, 0, this->width - 1);
		y = hclamp(y, 0, this->height - 1);
		sx = hclamp(sx, 0, dataWidth - 1);
		sy = hclamp(sy, 0, dataHeight - 1);
		sw = hmin(sw, hmin(this->width - x, dataWidth - sx));
		sh = hmin(sh, hmin(this->height - y, dataHeight - sy));
		unsigned char* p = this->manualData + (y * this->width + x) * this->bpp;
		this->_blit(p, x, y, data, dataWidth, dataHeight, dataBpp, sx, sy, sw, sh, alpha);
		this->_updateTexture();
	}
	
	void DirectX11_Texture::stretchBlit(int x, int y, int w, int h, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		DirectX11_Texture* source = (DirectX11_Texture*)texture;
		x = hclamp(x, 0, this->width - 1);
		y = hclamp(y, 0, this->height - 1);
		w = hmin(w, this->width - x);
		h = hmin(h, this->height - y);
		sx = hclamp(sx, 0, source->width - 1);
		sy = hclamp(sy, 0, source->height - 1);
		sw = hmin(sw, source->width - sx);
		sh = hmin(sh, source->height - sy);
		unsigned char* s = source->manualData + (sy * source->width + sx) * source->bpp;
		this->stretchBlit(x, y, w, h, s, source->width, source->height, source->bpp, sx, sy, sw, sh, alpha);
	}
	
	void DirectX11_Texture::stretchBlit(int x, int y, int w, int h, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		x = hclamp(x, 0, this->width - 1);
		y = hclamp(y, 0, this->height - 1);
		w = hmin(w, this->width - x);
		h = hmin(h, this->height - y);
		sx = hclamp(sx, 0, dataWidth - 1);
		sy = hclamp(sy, 0, dataHeight - 1);
		sw = hmin(sw, dataWidth - sx);
		sh = hmin(sh, dataHeight - sy);
		unsigned char* p = this->manualData + (y * this->width + x) * this->bpp;
		this->_stretchBlit(p, x, y, w, h, data, dataWidth, dataHeight, dataBpp, sx, sy, sw, sh, alpha);
		this->_updateTexture();
	}
	
	void DirectX11_Texture::rotateHue(float degrees)
	{
		if (degrees == 0.0f)
		{
			return;
		}
		int size = this->getByteSize();
		float range = hmodf(degrees, 360.0f) / 360.0f;
		float h;
		float s;
		float l;
		unsigned char* data = this->manualData;
		for_iter_step (i, 0, size, this->bpp)
		{
			april::rgbToHsl(data[i + 2], data[i + 1], data[i], &h, &s, &l);
			april::hslToRgb(hmodf(h + range, 1.0f), s, l, &data[i + 2], &data[i + 1], &data[i]);
		}
		this->_updateTexture();
	}
	
	void DirectX11_Texture::saturate(float factor)
	{
		int size = this->getByteSize();
		float h;
		float s;
		float l;
		unsigned char* data = this->manualData;
		for_iter_step (i, 0, size, this->bpp)
		{
			april::rgbToHsl(data[i + 2], data[i + 1], data[i], &h, &s, &l);
			april::hslToRgb(h, hmin(s * factor, 1.0f), l, &data[i + 2], &data[i + 1], &data[i]);
		}
		this->_updateTexture();
	}
	
	bool DirectX11_Texture::copyPixelData(unsigned char** output)
	{
		unsigned char* p = this->manualData;
		int i;
		int j;
		int offset;
		*output = new unsigned char[this->width * this->height * 4];
		if (this->bpp == 4)
		{
			memcpy(*output, p, this->getByteSize());
			for_iterx (j, 0, this->height)
			{
				for_iterx (i, 0, this->width)
				{
					offset = (j * this->width + i) * 4;
					(*output)[offset + 0] = p[offset + 2];
					//(*output)[offset + 1] = p[offset + 1]; // not necessary to be executed because of previous memcpy call
					(*output)[offset + 2] = p[offset + 0];
				}
			}
		}
		else if (this->bpp == 3)
		{
			for_iterx (j, 0, this->height)
			{
				for_iterx (i, 0, this->width)
				{
					offset = (j * this->width + i) * 4;
					(*output)[offset + 0] = p[offset + 2];
					(*output)[offset + 1] = p[offset + 1];
					(*output)[offset + 2] = p[offset + 0];
				}
			}
		}
		else if (this->bpp == 1)
		{
			memcpy(*output, p, this->getByteSize());
		}
		return true;
	}
	
	void DirectX11_Texture::insertAsAlphaMap(Texture* texture, unsigned char median, int ambiguity)
	{
		if (this->width != texture->getWidth() || this->height != texture->getHeight() || this->bpp != 4)
		{
			return;
		}
		DirectX11_Texture* source = (DirectX11_Texture*)texture;
		unsigned char* thisData = this->manualData;
		unsigned char* srcData = source->manualData;
		unsigned char* c;
		unsigned char* sc;
		int i;
		int j;
		int alpha;
		int min = (int)median - ambiguity / 2;
		int max = (int)median + ambiguity / 2;
		for_iterx (j, 0, this->height)
		{
			for_iterx (i, 0, this->width)
			{
				c = &thisData[(i + j * this->width) * 4];
				sc = &srcData[(i + j * this->width) * 4];
				alpha = (sc[0] + sc[1] + sc[2]) / 3;
				if (alpha < min)
				{
					c[3] = 255;
				}
				else if (alpha >= max)
				{
					c[3] = 0;
				}
				else
				{
					c[3] = (max - alpha) * 255 / ambiguity;
				}
			}
		}
		this->_updateTexture();
	}
	
	void DirectX11_Texture::write(int x, int y, unsigned char* data, int dataWidth, int dataHeight, int dataBpp)
	{
		if (this->bpp != dataBpp)
		{
			hlog::errorf(april::logTag, "Texture '%s' does not have same BPP (%d) as source data (%d)!", this->filename.c_str(), this->bpp, dataBpp);
			return;
		}
		x = hclamp(x, 0, this->width - 1);
		y = hclamp(y, 0, this->height - 1);
		if (x == 0 && dataWidth == this->width)
		{
			memcpy(this->manualData + (y * this->width) * this->bpp, data, dataWidth * dataHeight * dataBpp);
		}
		else
		{
			int w = hmin(this->width - x, dataWidth);
			for_iter (j, 0, dataHeight)
			{
				memcpy(this->manualData + ((y + j) * this->width + x) * this->bpp, data + (j * dataWidth) * dataBpp, w * dataBpp);
			}
		}
		this->_updateTexture();
	}
	
	void DirectX11_Texture::_updateTexture()
	{
		static D3D11_MAPPED_SUBRESOURCE mappedSubResource;
		APRIL_D3D_DEVICE_CONTEXT->Map(this->d3dTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubResource);
		unsigned int size = (unsigned int)this->width * this->height * this->bpp;
		if (mappedSubResource.RowPitch == size)
		{
			memcpy(mappedSubResource.pData, this->manualData, this->width * this->height * this->bpp);
		}
		else // DX11 textures don't work properly on some hardware and have a pitch different than w*bpp so this is needed
		{
			unsigned char* data = (unsigned char*)mappedSubResource.pData;
			int pitch = this->width * this->bpp;
			for_iter (j, 0, this->height)
			{
				memcpy(data + (j * mappedSubResource.RowPitch), this->manualData + (j * pitch), pitch);
			}
		}
		APRIL_D3D_DEVICE_CONTEXT->Unmap(this->d3dTexture.Get(), 0);
	}

}
#endif
