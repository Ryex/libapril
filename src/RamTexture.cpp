/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Color.h"
#include "Image.h"
#include "RamTexture.h"
#include "RenderSystem.h"

namespace april
{
	// TODO - this entire class needs to be removed
	RamTexture::RamTexture(chstr filename) : Texture(true)
	{
		this->filename = filename;
		this->source = NULL;
		hlog::write(april::logTag, "Creating RAM texture.");
	}

	RamTexture::RamTexture(int w, int h) : Texture(true)
	{
		this->width = w;
		this->height = h;
		this->source = Image::create(w, h, Color::Clear, Image::FORMAT_RGBA);
		this->format = this->source->format;
	}

	RamTexture::~RamTexture()
	{
		this->unload();
	}

	bool RamTexture::load()
	{
		if (this->source == NULL)
		{
			hlog::writef(april::logTag, "Loading RAM texture '%s'.", this->_getInternalName().c_str());
			if (this->filename != "")
			{
				this->source = Image::createFromResource(this->filename);
				this->width = this->source->w;
				this->height = this->source->h;
			}
			else
			{
				this->source = Image::create(this->width, this->height, Color::Clear, Image::FORMAT_RGBA);
			}
			this->format = this->source->format;
			return true;
		}
		return false;
	}
	
	void RamTexture::unload()
	{
		if (this->source != NULL)
		{
			hlog::writef(april::logTag, "Unloading RAM texture '%s'.", this->_getInternalName().c_str());
			delete this->source;
			this->source = NULL;
		}
	}
	
	bool RamTexture::isLoaded()
	{
		return (this->source != NULL);
	}
	
	Color RamTexture::getPixel(int x, int y)
	{
		if (this->source == NULL)
		{
			this->load();
		}
		return this->source->getPixel(x, y);
	}
	
	bool RamTexture::setPixel(int x, int y, Color color)
	{
		if (this->source == NULL)
		{
			this->load();
		}
		return this->source->setPixel(x, y, color);
	}

	bool RamTexture::_createInternalTexture(unsigned char* data, int size, Type type)
	{
		return true;
	}

	void RamTexture::_assignFormat()
	{
	}

	Texture::Lock RamTexture::_tryLockSystem(int x, int y, int w, int h)
	{
		return Lock();
	}

	bool RamTexture::_unlockSystem(Lock& lock, bool update)
	{
		return true;
	}

	bool RamTexture::_uploadToGpu(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		return true;
	}

	bool RamTexture::write(int sx, int sy, int sw, int sh, int dx, int dy, Texture* texture)
	{
		return false;
	}

	bool RamTexture::writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Texture* texture)
	{
		return false;
	}

	bool RamTexture::blit(int sx, int sy, int sw, int sh, int dx, int dy, Texture* texture, unsigned char alpha)
	{
		return false;
	}

	bool RamTexture::blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Texture* texture, unsigned char alpha)
	{
		return false;
	}

	bool RamTexture::insertAlphaMap(Texture* texture, unsigned char median, int ambiguity)
	{
		return false;
	}
		
}
