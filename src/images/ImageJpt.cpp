/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hsbase.h>
#include <hltypes/hstream.h>

#include "Image.h"

namespace april
{
	Image* Image::_loadJpt(hsbase& stream)
	{
		Image* jpg = NULL;
		Image* png = NULL;
		unsigned char bytes[4] = {0};
		// file header ("JPT" + 1 byte for version code)
		stream.read_raw(bytes, 4);
		// read JPEG
		stream.read_raw(bytes, 4);
		jpg = Image::_loadJpg(stream, bytes[0] + (bytes[1] << 8) + (bytes[2] << 16) + (bytes[3] << 24));
		// read PNG
		stream.read_raw(bytes, 4);
		png = Image::_loadPng(stream, bytes[0] + (bytes[1] << 8) + (bytes[2] << 16) + (bytes[3] << 24));
		png->format = FORMAT_ALPHA;
		// combine
		Image* image = Image::create(jpg->w, jpg->h, Color::Clear, FORMAT_RGBA);
		image->write(0, 0, jpg->w, jpg->h, 0, 0, jpg);
		image->write(0, 0, png->w, png->h, 0, 0, png);
		delete jpg;
		delete png;
		return image;
	}

}
