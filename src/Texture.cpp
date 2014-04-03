/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 3.31
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/harray.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Color.h"
#include "Image.h"
#include "Texture.h"
#include "RenderSystem.h"

#define HROUND_GRECT(rect) hround(rect.x), hround(rect.y), hround(rect.w), hround(rect.h)
#define HROUND_GVEC2(vec2) hround(vec2.x), hround(vec2.y)

namespace april
{
	Image::Format Texture::FORMAT_ALPHA = Image::FORMAT_ALPHA; // DEPRECATED
	Image::Format Texture::FORMAT_ARGB = Image::FORMAT_RGBA; // DEPRECATED

	Texture::Lock::Lock()
	{
		this->systemBuffer = NULL;
		this->x = 0;
		this->y = 0;
		this->w = 0;
		this->h = 0;
		this->dx = 0;
		this->dy = 0;
		this->data = NULL;
		this->dataWidth = 0;
		this->dataHeight = 0;
		this->format = Image::FORMAT_INVALID;
		this->locked = false;
		this->failed = true;
		this->renderTarget = false;
	}

	Texture::Lock::~Lock()
	{
	}

	void Texture::Lock::activateFail()
	{
		this->locked = false;
		this->failed = true;
		this->renderTarget = false;
	}

	void Texture::Lock::activateLock(int x, int y, int w, int h, int dx, int dy, unsigned char* data, int dataWidth, int dataHeight, Image::Format format)
	{
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
		this->dx = dx;
		this->dy = dy;
		this->data = data;
		this->dataWidth = dataWidth;
		this->dataHeight = dataHeight;
		this->format = format;
		this->locked = true;
		this->failed = false;
		this->renderTarget = false;
	}

	void Texture::Lock::activateRenderTarget(int x, int y, int w, int h, int dx, int dy, unsigned char* data, int dataWidth, int dataHeight, Image::Format format)
	{
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
		this->dx = dx;
		this->dy = dy;
		this->data = data;
		this->dataWidth = dataWidth;
		this->dataHeight = dataHeight;
		this->format = format;
		this->locked = false;
		this->failed = false;
		this->renderTarget = true;
	}

	Texture::Texture(bool fromResource)
	{
		this->filename = "";
		this->type = TYPE_IMMUTABLE;
		this->format = Image::FORMAT_INVALID;
		this->dataFormat = 0;
		this->width = 0;
		this->height = 0;
		this->filter = FILTER_LINEAR;
		this->addressMode = ADDRESS_WRAP;
		this->data = NULL;
		this->fromResource = fromResource;
		april::rendersys->textures += this;
	}

	bool Texture::_create(chstr filename, Texture::Type type)
	{
		this->filename = filename;
		this->type = type;
		this->width = 0;
		this->height = 0;
		this->type = type;
		this->format = Image::FORMAT_INVALID;
		this->dataFormat = 0;
		this->data = NULL;
		hlog::write(april::logTag, "Creating texture: " + this->_getInternalName());
		return true;
	}

	bool Texture::_create(chstr filename, Image::Format format, Texture::Type type)
	{
		this->filename = filename;
		this->type = type;
		this->width = 0;
		this->height = 0;
		this->type = type;
		this->format = format;
		this->dataFormat = 0;
		this->data = NULL;
		hlog::write(april::logTag, "Creating texture: " + this->_getInternalName());
		return true;
	}

	bool Texture::_create(int w, int h, unsigned char* data, Image::Format format, Texture::Type type)
	{
		if (w == 0 || h == 0)
		{
			hlog::errorf(april::logTag, "Cannot create texture with dimentions %d,%d!", w, h);
			return false;
		}
		this->filename = "";
		this->width = w;
		this->height = h;
		this->type = TYPE_VOLATILE; // so the write call later on goes through
		int size = 0;
		if (type != TYPE_VOLATILE)
		{
			this->format = format;
			size = this->getByteSize();
			this->data = new unsigned char[size];
			this->type = TYPE_MANAGED;
		}
		else
		{
			this->format = april::rendersys->getNativeTextureFormat(format);
			size = this->getByteSize();
		}
		hlog::write(april::logTag, "Creating texture: " + this->_getInternalName());
		this->dataFormat = 0;
		this->_assignFormat();
		if (!this->_createInternalTexture(data, size, type))
		{
			return false;
		}
		this->write(0, 0, this->width, this->height, 0, 0, data, this->width, this->height, format);
		this->type = type;
		return true;
	}

	bool Texture::_create(int w, int h, Color color, Image::Format format, Texture::Type type)
	{
		if (w == 0 || h == 0)
		{
			hlog::errorf(april::logTag, "Cannot create texture with dimensions %d,%d!", w, h);
			return false;
		}
		this->filename = "";
		this->width = w;
		this->height = h;
		this->type = TYPE_VOLATILE; // so the write call later on goes through
		int size = 0;
		if (type != TYPE_VOLATILE)
		{
			this->format = format;
			size = this->getByteSize();
			this->data = new unsigned char[size];
			this->type = TYPE_MANAGED;
		}
		else
		{
			this->format = april::rendersys->getNativeTextureFormat(format);
			size = this->getByteSize();
		}
		hlog::write(april::logTag, "Creating texture: " + this->_getInternalName());
		this->dataFormat = 0;
		this->_assignFormat();
		if (!this->_createInternalTexture(this->data, size, type))
		{
			return false;
		}
		this->fillRect(0, 0, this->width, this->height, color);
		this->type = type;
		return true;
	}

	Texture::~Texture()
	{
		april::rendersys->textures -= this;
		if (this->data != NULL)
		{
			delete this->data;
		}
	}

	int Texture::getWidth()
	{
		if (this->width == 0)
		{
			hlog::warnf(april::logTag, "Texture '%s' has width = 0 (possibly not loaded yet?)", this->filename.c_str());
		}
		return this->width;
	}

	int Texture::getHeight()
	{
		if (this->height == 0)
		{
			hlog::warnf(april::logTag, "Texture '%s' has height = 0 (possibly not loaded yet?)", this->filename.c_str());
		}
		return this->height;
	}

	int Texture::getBpp()
	{
		if (this->format == Image::FORMAT_INVALID)
		{
			hlog::warnf(april::logTag, "Texture '%s' has bpp = 0 (possibly not loaded yet?)", this->filename.c_str());
		}
		return Image::getFormatBpp(this->format);
	}

	int Texture::getByteSize()
	{
		if (this->width == 0 || this->height == 0 || this->format == Image::FORMAT_INVALID)
		{
			hlog::warnf(april::logTag, "Texture '%s' has byteSize = 0 (possibly not loaded yet?)", this->filename.c_str());
		}
		return (this->width * this->height * Image::getFormatBpp(this->format));
	}

	hstr Texture::_getInternalName()
	{
		hstr result;
		if (this->filename != "")
		{
			result += "'" + this->filename + "'";
		}
		else
		{
			result += hsprintf("<0x%p>", this);
		}
		switch (this->type)
		{
		case TYPE_IMMUTABLE:
			result += " (immutable)";
			break;
		case TYPE_MANAGED:
			result += " (managed)";
			break;
		case TYPE_VOLATILE:
			result += " (volatile)";
			break;
		}
		return result;
	}

	bool Texture::load()
	{
		if (this->isLoaded())
		{
			return true;
		}
		hlog::write(april::logTag, "Loading texture: " + this->_getInternalName());
		int size = 0;
		unsigned char* currentData = NULL;
		if (this->data != NULL) // reload from memory
		{
			currentData = this->data;
			size = this->getByteSize();
		}
		// if no cached data and not a volatile texture that was previously loaded and thus has a width and height
		if (currentData == NULL && (type != TYPE_VOLATILE || this->width == 0 || this->height == 0))
		{
			if (this->filename == "")
			{
				hlog::error(april::logTag, "No filename for texture specified!");
				return false;
			}
			Image* image = NULL;
			if (this->format == Image::FORMAT_INVALID)
			{
				image = (this->fromResource ? Image::createFromResource(this->filename) : Image::createFromFile(this->filename));
			}
			else
			{
				image = (this->fromResource ? Image::createFromResource(this->filename, this->format) : Image::createFromFile(this->filename, this->format));
			}
			if (image == NULL)
			{
				hlog::error(april::logTag, "Failed to load texture: " + this->_getInternalName());
				return false;
			}
			this->width = image->w;
			this->height = image->h;
			this->format = image->format;
			this->dataFormat = image->internalFormat;
			if (this->dataFormat != 0)
			{
				size = image->compressedSize;
			}
			currentData = image->data;
			image->data = NULL;
			delete image;
		}
		this->_assignFormat();
		if (!this->_createInternalTexture(currentData, size, this->type))
		{
			if (currentData != NULL && this->data != currentData)
			{
				delete [] currentData;
			}
			return false;
		}
		if (currentData != NULL)
		{
			Type type = this->type;
			this->type = TYPE_VOLATILE; // so the write call right below goes through
			this->write(0, 0, this->width, this->height, 0, 0, currentData, this->width, this->height, format);
			this->type = type;
			if (this->type != TYPE_VOLATILE && (this->type != TYPE_IMMUTABLE || this->filename == ""))
			{
				if (this->data != currentData)
				{
					if (this->data != NULL)
					{
						delete [] this->data;
					}
					this->data = currentData;
				}
			}
			else
			{
				delete [] currentData;
				// the used format will be the native format, because there is no intermediate data
				this->format = april::rendersys->getNativeTextureFormat(this->format);
			}
		}
		return true;
	}

	bool Texture::clear()
	{
		if (this->type == TYPE_IMMUTABLE)
		{
			hlog::warn(april::logTag, "Cannot write texture: " + this->_getInternalName());
			return false;
		}
		Lock lock = this->_tryLock();
		if (lock.failed)
		{
			return false;
		}
		memset(lock.data, 0, this->getByteSize());
		return this->_unlock(lock, true);
	}

	Color Texture::getPixel(int x, int y)
	{
		Color color = Color::Clear;
		if (this->type != TYPE_MANAGED)
		{
			hlog::warn(april::logTag, "Cannot read texture: " + this->_getInternalName());
			return color;
		}
		if (this->data != NULL)
		{
			color = Image::getPixel(x, y, this->data, this->width, this->height, this->format);
		}
		return color;
	}

	bool Texture::setPixel(int x, int y, Color color)
	{
		if (this->type == TYPE_IMMUTABLE)
		{
			hlog::warn(april::logTag, "Cannot write texture: " + this->_getInternalName());
			return false;
		}
		Lock lock = this->_tryLock(x, y, 1, 1);
		if (lock.failed)
		{
			return false;
		}
		return this->_unlock(lock, Image::setPixel(lock.x, lock.y, color, lock.data, lock.dataWidth, lock.dataHeight, lock.format));
	}

	Color Texture::getInterpolatedPixel(float x, float y)
	{
		Color color = Color::Clear;
		if (this->type != TYPE_MANAGED)
		{
			hlog::warn(april::logTag, "Cannot read texture: " + this->_getInternalName());
			return color;
		}
		if (this->data != NULL)
		{
			color = Image::getInterpolatedPixel(x, y, this->data, this->width, this->height, this->format);
		}
		return color;
	}

	bool Texture::fillRect(int x, int y, int w, int h, Color color)
	{
		if (this->type == TYPE_IMMUTABLE)
		{
			hlog::warn(april::logTag, "Cannot write texture: " + this->_getInternalName());
			return false;
		}
		if (w == 1 && h == 1)
		{
			return this->setPixel(x, y, color);
		}
		Lock lock = this->_tryLock(x, y, w, h);
		if (lock.failed)
		{
			return false;
		}
		return this->_unlock(lock, Image::fillRect(lock.x, lock.y, lock.w, lock.h, color, lock.data, lock.dataWidth, lock.dataHeight, lock.format));
	}

	bool Texture::copyPixelData(unsigned char** output, Image::Format format)
	{
		if (this->type != TYPE_MANAGED)
		{
			hlog::warn(april::logTag, "Cannot read texture: " + this->_getInternalName());
			return false;
		}
		if (!this->isLoaded())
		{
			return false;
		}
		Lock lock = this->_tryLock(); // will use this->data, no need for native format checking and direct copying
		if (lock.failed)
		{
			return false;
		}
		bool result = Image::convertToFormat(lock.dataWidth, lock.dataHeight, lock.data, lock.format, output, format, false);
		this->_unlock(lock, false);
		return result;
	}

	bool Texture::write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		if (this->type == TYPE_IMMUTABLE)
		{
			hlog::warn(april::logTag, "Cannot write texture: " + this->_getInternalName());
			return false;
		}
		if (this->type == TYPE_VOLATILE && !Image::needsConversion(srcFormat, april::rendersys->getNativeTextureFormat(this->format)) &&
			this->_uploadToGpu(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat))
		{
			return true;
		}
		Lock lock = this->_tryLock(dx, dy, sw, sh);
		if (lock.failed)
		{
			return false;
		}
		return this->_unlock(lock, Image::write(sx, sy, sw, sh, lock.x, lock.y, srcData, srcWidth, srcHeight, srcFormat, lock.data, lock.dataWidth, lock.dataHeight, lock.format));
	}

	bool Texture::write(int sx, int sy, int sw, int sh, int dx, int dy, Texture* texture)
	{
		if (this->type == TYPE_IMMUTABLE)
		{
			hlog::warn(april::logTag, "Cannot write texture: " + this->_getInternalName());
			return false;
		}
		if (texture->type != TYPE_MANAGED)
		{
			hlog::warn(april::logTag, "Cannot read texture: " + texture->_getInternalName());
			return false;
		}
		if (texture == NULL || !texture->isLoaded())
		{
			return false;
		}
		Lock lock = texture->_tryLock(sx, sy, sw, sh);
		if (lock.failed)
		{
			return false;
		}
		bool result = this->write(lock.dx, lock.dy, lock.w, lock.h, dx, dy, lock.data, lock.dataWidth, lock.dataHeight, lock.format);
		texture->_unlock(lock, false);
		return result;
	}

	bool Texture::writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		if (this->type == TYPE_IMMUTABLE)
		{
			hlog::warn(april::logTag, "Cannot write texture: " + this->_getInternalName());
			return false;
		}
		Lock lock = this->_tryLock(dx, dy, dw, dh);
		if (lock.failed)
		{
			return false;
		}
		return this->_unlock(lock, Image::writeStretch(sx, sy, sw, sh, lock.x, lock.y, lock.w, lock.h, srcData, srcWidth, srcHeight, srcFormat, lock.data, lock.dataWidth, lock.dataHeight, lock.format));
	}

	bool Texture::writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Texture* texture)
	{
		if (this->type == TYPE_IMMUTABLE)
		{
			hlog::warn(april::logTag, "Cannot write texture: " + this->_getInternalName());
			return false;
		}
		if (texture->type != TYPE_MANAGED)
		{
			hlog::warn(april::logTag, "Cannot read texture: " + texture->_getInternalName());
			return false;
		}
		if (texture == NULL || !texture->isLoaded())
		{
			return false;
		}
		Lock lock = texture->_tryLock(sx, sy, sw, sh);
		if (lock.failed)
		{
			return false;
		}
		bool result = this->writeStretch(lock.dx, lock.dy, lock.w, lock.h, dx, dy, dw, dh, lock.data, lock.dataWidth, lock.dataHeight, lock.format);
		texture->_unlock(lock, false);
		return result;
	}

	bool Texture::blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		if (this->type != TYPE_MANAGED)
		{
			hlog::warn(april::logTag, "Cannot alter texture: " + this->_getInternalName());
			return false;
		}
		Lock lock = this->_tryLock(dx, dy, sw, sh);
		if (lock.failed)
		{
			return false;
		}
		return this->_unlock(lock, Image::blit(sx, sy, sw, sh, lock.x, lock.y, srcData, srcWidth, srcHeight, srcFormat, lock.data, lock.dataWidth, lock.dataHeight, lock.format, alpha));
	}

	bool Texture::blit(int sx, int sy, int sw, int sh, int dx, int dy, Texture* texture, unsigned char alpha)
	{
		if (this->type != TYPE_MANAGED)
		{
			hlog::warn(april::logTag, "Cannot alter texture: " + this->_getInternalName());
			return false;
		}
		if (texture->type != TYPE_MANAGED)
		{
			hlog::warn(april::logTag, "Cannot read texture: " + texture->_getInternalName());
			return false;
		}
		if (texture == NULL || !texture->isLoaded())
		{
			return false;
		}
		Lock lock = texture->_tryLock(sx, sy, sw, sh);
		if (lock.failed)
		{
			return false;
		}
		bool result = this->blit(lock.dx, lock.dy, lock.w, lock.h, dx, dy, lock.data, lock.dataWidth, lock.dataHeight, lock.format, alpha);
		texture->_unlock(lock, false);
		return result;
	}

	bool Texture::blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		if (this->type != TYPE_MANAGED)
		{
			hlog::warn(april::logTag, "Cannot alter texture: " + this->_getInternalName());
			return false;
		}
		Lock lock = this->_tryLock(dx, dy, dw, dh);
		if (lock.failed)
		{
			return false;
		}
		return this->_unlock(lock, Image::blitStretch(sx, sy, sw, sh, lock.x, lock.y, lock.w, lock.h, srcData, srcWidth, srcHeight, srcFormat, lock.data, lock.dataWidth, lock.dataHeight, lock.format, alpha));
	}

	bool Texture::blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Texture* texture, unsigned char alpha)
	{
		if (this->type != TYPE_MANAGED)
		{
			hlog::warn(april::logTag, "Cannot alter texture: " + this->_getInternalName());
			return false;
		}
		if (texture->type != TYPE_MANAGED)
		{
			hlog::warn(april::logTag, "Cannot read texture: " + texture->_getInternalName());
			return false;
		}
		if (texture == NULL || !texture->isLoaded())
		{
			return false;
		}
		Lock lock = texture->_tryLock(sx, sy, sw, sh);
		if (lock.failed)
		{
			return false;
		}
		bool result = this->blitStretch(lock.dx, lock.dy, lock.w, lock.h, dx, dy, dw, dh, lock.data, lock.dataWidth, lock.dataHeight, lock.format, alpha);
		texture->_unlock(lock, false);
		return result;
	}

	bool Texture::rotateHue(int x, int y, int w, int h, float degrees)
	{
		if (this->type != TYPE_MANAGED)
		{
			hlog::warn(april::logTag, "Cannot alter texture: " + this->_getInternalName());
			return false;
		}
		Lock lock = this->_tryLock(x, y, w, h);
		if (lock.failed)
		{
			return false;
		}
		return this->_unlock(lock, Image::rotateHue(lock.x, lock.y, lock.w, lock.h, degrees, lock.data, lock.dataWidth, lock.dataHeight, lock.format));
	}

	bool Texture::saturate(int x, int y, int w, int h, float factor)
	{
		if (this->type != TYPE_MANAGED)
		{
			hlog::warn(april::logTag, "Cannot alter texture: " + this->_getInternalName());
			return false;
		}
		Lock lock = this->_tryLock(x, y, w, h);
		if (lock.failed)
		{
			return false;
		}
		return this->_unlock(lock, Image::saturate(lock.x, lock.y, lock.w, lock.h, factor, lock.data, lock.dataWidth, lock.dataHeight, lock.format));
	}

	bool Texture::invert(int x, int y, int w, int h)
	{
		if (this->type != TYPE_MANAGED)
		{
			hlog::warn(april::logTag, "Cannot alter texture: " + this->_getInternalName());
			return false;
		}
		Lock lock = this->_tryLock(x, y, w, h);
		if (lock.failed)
		{
			return false;
		}
		return this->_unlock(lock, Image::invert(lock.x, lock.y, lock.w, lock.h, lock.data, lock.dataWidth, lock.dataHeight, lock.format));
	}

	bool Texture::insertAlphaMap(unsigned char* srcData, Image::Format srcFormat, unsigned char median, int ambiguity)
	{
		if (this->type != TYPE_MANAGED)
		{
			hlog::warn(april::logTag, "Cannot alter texture: " + this->_getInternalName());
			return false;
		}
		Lock lock = this->_tryLock();
		if (lock.failed)
		{
			return false;
		}
		return this->_unlock(lock, Image::insertAlphaMap(lock.dataWidth, lock.dataHeight, srcData, srcFormat, lock.data, lock.format, median, ambiguity));
	}

	bool Texture::insertAlphaMap(Texture* texture, unsigned char median, int ambiguity)
	{
		if (this->type != TYPE_MANAGED)
		{
			hlog::warn(april::logTag, "Cannot alter texture: " + this->_getInternalName());
			return false;
		}
		if (texture->type != TYPE_MANAGED)
		{
			hlog::warn(april::logTag, "Cannot read texture: " + texture->_getInternalName());
			return false;
		}
		if (texture == NULL || !texture->isLoaded() || texture->width != this->width || texture->height != this->height)
		{
			return false;
		}
		Lock lock = texture->_tryLock();
		if (lock.failed)
		{
			return false;
		}
		bool result = this->insertAlphaMap(lock.data, lock.format, median, ambiguity);
		texture->_unlock(lock, false);
		return result;
	}

	// overloads

	Color Texture::getPixel(gvec2 position)
	{
		return this->getPixel(HROUND_GVEC2(position));
	}

	bool Texture::setPixel(gvec2 position, Color color)
	{
		return this->setPixel(HROUND_GVEC2(position), color);
	}

	Color Texture::getInterpolatedPixel(gvec2 position)
	{
		return this->getInterpolatedPixel(position.x, position.y);
	}

	bool Texture::fillRect(grect rect, Color color)
	{
		return this->fillRect(HROUND_GRECT(rect), color);
	}

	bool Texture::copyPixelData(unsigned char** output)
	{
		return this->copyPixelData(output, this->format);
	}

	bool Texture::write(grect srcRect, gvec2 destPosition, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		return this->write(HROUND_GRECT(srcRect), HROUND_GVEC2(destPosition), srcData, srcWidth, srcHeight, srcFormat);
	}

	bool Texture::write(grect srcRect, gvec2 destPosition, Texture* texture)
	{
		return this->write(HROUND_GRECT(srcRect), HROUND_GVEC2(destPosition), texture);
	}

	bool Texture::write(int sx, int sy, int sw, int sh, int dx, int dy, Image* image)
	{
		return this->write(sx, sy, sw, sh, dx, dy, image->data, image->w, image->h, image->format);
	}

	bool Texture::write(grect srcRect, gvec2 destPosition, Image* image)
	{
		return this->write(HROUND_GRECT(srcRect), HROUND_GVEC2(destPosition), image->data, image->w, image->h, image->format);
	}

	bool Texture::writeStretch(grect srcRect, grect destRect, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		return this->writeStretch(HROUND_GRECT(srcRect), HROUND_GRECT(destRect), srcData, srcWidth, srcHeight, srcFormat);
	}

	bool Texture::writeStretch(grect srcRect, grect destRect, Texture* texture)
	{
		return this->writeStretch(HROUND_GRECT(srcRect), HROUND_GRECT(destRect), texture);
	}

	bool Texture::writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Image* image)
	{
		return this->writeStretch(sx, sy, sw, sh, dx, dy, dw, dh, image->data, image->w, image->h, image->format);
	}

	bool Texture::writeStretch(grect srcRect, grect destRect, Image* image)
	{
		return this->writeStretch(HROUND_GRECT(srcRect), HROUND_GRECT(destRect), image->data, image->w, image->h, image->format);
	}

	bool Texture::blit(grect srcRect, gvec2 destPosition, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		return this->blit(HROUND_GRECT(srcRect), HROUND_GVEC2(destPosition), srcData, srcWidth, srcHeight, srcFormat, alpha);
	}

	bool Texture::blit(grect srcRect, gvec2 destPosition, Texture* texture, unsigned char alpha)
	{
		return this->blit(HROUND_GRECT(srcRect), HROUND_GVEC2(destPosition), texture, alpha);
	}

	bool Texture::blit(int sx, int sy, int sw, int sh, int dx, int dy, Image* image, unsigned char alpha)
	{
		return this->blit(sx, sy, sw, sh, dx, dy, image->data, image->w, image->h, image->format, alpha);
	}

	bool Texture::blit(grect srcRect, gvec2 destPosition, Image* image, unsigned char alpha)
	{
		return this->blit(HROUND_GRECT(srcRect), HROUND_GVEC2(destPosition), image->data, image->w, image->h, image->format, alpha);
	}

	bool Texture::blitStretch(grect srcRect, grect destRect, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		return this->blitStretch(HROUND_GRECT(srcRect), HROUND_GRECT(destRect), srcData, srcWidth, srcHeight, srcFormat, alpha);
	}

	bool Texture::blitStretch(grect srcRect, grect destRect, Texture* texture, unsigned char alpha)
	{
		return this->blitStretch(HROUND_GRECT(srcRect), HROUND_GRECT(destRect), texture, alpha);
	}

	bool Texture::blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Image* image, unsigned char alpha)
	{
		return this->blitStretch(sx, sy, sw, sh, dx, dy, dw, dh, image->data, image->w, image->h, image->format, alpha);
	}

	bool Texture::blitStretch(grect srcRect, grect destRect, Image* image, unsigned char alpha)
	{
		return this->blitStretch(HROUND_GRECT(srcRect), HROUND_GRECT(destRect), image->data, image->w, image->h, image->format, alpha);
	}

	bool Texture::rotateHue(grect rect, float degrees)
	{
		return this->rotateHue(HROUND_GRECT(rect), degrees);
	}

	bool Texture::saturate(grect rect, float factor)
	{
		return this->saturate(HROUND_GRECT(rect), factor);
	}

	bool Texture::invert(grect rect)
	{
		return this->invert(HROUND_GRECT(rect));
	}

	bool Texture::insertAlphaMap(Image* image, unsigned char median, int ambiguity)
	{
		if (image->w != this->width || image->h != this->height)
		{
			return false;
		}
		return this->insertAlphaMap(image->data, image->format, median, ambiguity);
	}

	Texture::Lock Texture::_tryLock(int x, int y, int w, int h)
	{
		Lock lock;
		if (this->data != NULL)
		{
			lock.activateLock(x, y, w, h, x, y, this->data, this->width, this->height, this->format);
		}
		else
		{
			this->load();
			lock = this->_tryLockSystem(x, y, w, h);
		}
		return lock;
	}

	Texture::Lock Texture::_tryLock()
	{
		return this->_tryLock(0, 0, this->width, this->height);
	}

	bool Texture::_unlock(Texture::Lock lock, bool update)
	{
		if (!this->_unlockSystem(lock, update) && !lock.failed && update)
		{
			update = this->_uploadDataToGpu(lock.dx, lock.dy, lock.w, lock.h);
		}
		return update;
	}

	bool Texture::_uploadDataToGpu(int x, int y, int w, int h)
	{
		if (!Image::needsConversion(this->format, april::rendersys->getNativeTextureFormat(this->format)) &&
			this->_uploadToGpu(x, y, w, h, x, y, this->data, this->width, this->height, this->format))
		{
			return true;
		}
		this->load();
		Lock lock = this->_tryLockSystem(x, y, w, h);
		if (lock.failed)
		{
			return false;
		}
		bool result = Image::write(x, y, w, h, lock.x, lock.y, this->data, this->width, this->height, this->format, lock.data, lock.dataWidth, lock.dataHeight, lock.format);
		this->_unlockSystem(lock, true);
		return result;
	}

}
