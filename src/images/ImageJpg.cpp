/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <stdio.h>

#include <hltypes/hltypesUtil.h>
#include <hltypes/hresource.h>
#include <hltypes/hsbase.h>
#include <hltypes/hstream.h>

extern "C"
{
#include <jpeglib.h> // has to be down here because of problems with header order
}

#include "Image.h"

namespace april
{
	Image* Image::_loadJpg(hsbase& stream, int size)
	{
		// first read the whole data from the resource file
		unsigned char* compressedData = new unsigned char[size];
		stream.read_raw(compressedData, size);
		// read JPEG image from file data
		struct jpeg_decompress_struct cInfo;
		struct jpeg_error_mgr jErr;
		cInfo.err = jpeg_std_error(&jErr);
		jpeg_create_decompress(&cInfo);
		jpeg_mem_src(&cInfo, compressedData, size);
		jpeg_read_header(&cInfo, TRUE);
		jpeg_start_decompress(&cInfo);
		unsigned char* imageData = new unsigned char[cInfo.output_width * cInfo.output_height * 3]; // JPEG is always RGB
		unsigned char* ptr = NULL;
		for_itert (unsigned int, i, 0, cInfo.output_height)
		{
			ptr = imageData + i * cInfo.output_width * 3;
			jpeg_read_scanlines(&cInfo, &ptr, 1);
		}
		jpeg_finish_decompress(&cInfo);
		jpeg_destroy_decompress(&cInfo);
		delete [] compressedData;
		// assign Image data
		Image* image = new Image();
		image->data = imageData;
		image->w = cInfo.output_width;
		image->h = cInfo.output_height;
		image->format = Image::FORMAT_RGB; // JPEG is always RGB
		return image;
	}

	Image* Image::_loadJpg(hsbase& stream)
	{
		return Image::_loadJpg(stream, stream.size());
	}

}
