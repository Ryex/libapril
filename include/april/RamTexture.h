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
/// Defines a special RAM texture.

#ifndef APRIL_RAM_TEXTURE_H
#define APRIL_RAM_TEXTURE_H

#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Color.h"
#include "Texture.h"

namespace april
{
	class Image;
	
	class aprilExport RamTexture : public Texture
	{
	public:
		RamTexture(chstr filename);
		RamTexture(int w, int h);
		~RamTexture();
		bool load();
		void unload();

		bool isLoaded();

		Color getPixel(int x, int y);
		bool setPixel(int x, int y, Color color);

		bool write(int sx, int sy, int sw, int sh, int dx, int dy, Texture* texture);
		bool writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Texture* texture);
		bool blit(int sx, int sy, int sw, int sh, int dx, int dy, Texture* texture, unsigned char alpha = 255);
		bool blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Texture* texture, unsigned char alpha = 255);
		bool insertAlphaMap(Texture* texture, unsigned char median, int ambiguity);
		
	protected:
		Image* source;

		// not used
		bool _createInternalTexture(unsigned char* data, int size, Type type);
		void _assignFormat();
		Lock _tryLockSystem(int x, int y, int w, int h);
		bool _unlockSystem(Lock& lock, bool update);
		bool _uploadToGpu(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		
	};

}

#endif
