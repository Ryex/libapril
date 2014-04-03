/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.33
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <string.h>

#include <hltypes/hfile.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Color.h"
#include "Image.h"
#include "RenderSystem.h"

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#define CHECK_SHIFT_FORMATS(format1, format2) (\
	((format1) == FORMAT_RGBA || (format1) == FORMAT_RGBX || (format1) == FORMAT_BGRA || (format1) == FORMAT_BGRX) && \
	((format2) == FORMAT_ARGB || (format2) == FORMAT_XRGB || (format2) == FORMAT_ABGR || (format2) == FORMAT_XBGR) \
)
#define CHECK_INVERT_ORDER_FORMATS(format1, format2) (\
	((format1) == FORMAT_RGBA || (format1) == FORMAT_RGBX || (format1) == FORMAT_ARGB || (format1) == FORMAT_XRGB) && \
	((format2) == FORMAT_BGRA || (format2) == FORMAT_BGRX || (format2) == FORMAT_ABGR || (format2) == FORMAT_XBGR) \
)
#define CHECK_LEFT_RGB(format) \
	((format) == FORMAT_RGBA || (format) == FORMAT_RGBX || (format) == FORMAT_BGRA || (format) == FORMAT_BGRX)
#define CHECK_ALPHA_FORMAT(format) \
	((format) == FORMAT_RGBA || (format) == FORMAT_ARGB || (format) == FORMAT_BGRA || (format) == FORMAT_ABGR)

#define FOR_EACH_4BPP_PIXEL(macro) \
	for_iterx (y, 0, h) \
	{ \
		for_iterx (x, 0, w) \
		{ \
			i = (x + y * w); \
			dest[i] = macro(src[i]); \
		} \
	}
#define FOR_EACH_3BPP_TO_4BPP_PIXEL(exec) \
	for_iterx (y, 0, h) \
	{ \
		for_iterx (x, 0, w) \
		{ \
			i = (x + y * w) * srcBpp; \
			dest[x + y * w] = (exec); \
		} \
	}
#define FOR_EACH_4BPP_TO_3BPP_PIXEL(exec1, exec2, exec3) \
	for_iterx (y, 0, h) \
	{ \
		for_iterx (x, 0, w) \
		{ \
			i = x + y * w; \
			j = (x + y * w) * destBpp; \
			dest[j] = (unsigned char)(exec1); \
			dest[j + 1] = (unsigned char)(exec2); \
			dest[j + 2] = (unsigned char)(exec3); \
		} \
	}

// these all assume little endian
#define _R_ALPHA 0xFF
#define _L_ALPHA 0xFF000000
#define _KEEP_R(value) ((value) & 0xFFFFFF00)
#define _KEEP_L(value) ((value) & 0xFFFFFF)
#define _RIGHT_SHIFT(value) ((value) << 8)
#define _LEFT_SHIFT(value) ((value) >> 8)
#define _R_SHIFT_ALPHA(value) ((value) >> 24)
#define _L_SHIFT_ALPHA(value) ((value) << 24)
#define _INVERTED_R(value) ((((value) & 0xFF000000) >> 16) | (((value) & 0xFF00) << 16) | ((value) & 0xFF0000))
#define _INVERTED_L(value) ((((value) & 0xFF0000) >> 16) | (((value) & 0xFF) << 16) | ((value) & 0xFF00))
#define _INVERTED_RIGHT_SHIFT(value) (((value) << 24) | (((value) & 0xFF00) << 8) | (((value) & 0xFF0000) >> 8))
#define _INVERTED_LEFT_SHIFT(value) (((value) >> 24) | (((value) & 0xFF0000) >> 8) | (((value) & 0xFF00) << 8))

#define KEEP_R(value) (_KEEP_R(value) | _R_ALPHA)
#define KEEP_L(value) (_KEEP_L(value) | _L_ALPHA)
#define RIGHT_SHIFT(value) (_RIGHT_SHIFT(value) | _R_ALPHA)
#define RIGHT_SHIFT_WITH_ALPHA(value) (_RIGHT_SHIFT(value) | _R_SHIFT_ALPHA(value))
#define LEFT_SHIFT(value) (_LEFT_SHIFT(value) | _L_ALPHA)
#define LEFT_SHIFT_WITH_ALPHA(value) (_LEFT_SHIFT(value) | _L_SHIFT_ALPHA(value))
#define INVERTED_R(value) (_INVERTED_R(value) | _R_ALPHA)
#define INVERTED_R_WITH_ALPHA(value) (_INVERTED_R(value) | ((value) & _R_ALPHA))
#define INVERTED_L(value) (_INVERTED_L(value) | _L_ALPHA)
#define INVERTED_L_WITH_ALPHA(value) (_INVERTED_L(value) | ((value) & _L_ALPHA))
#define INVERTED_RIGHT_SHIFT(value) (_INVERTED_RIGHT_SHIFT(value) | _R_ALPHA)
#define INVERTED_RIGHT_SHIFT_WITH_ALPHA(value) (_INVERTED_RIGHT_SHIFT(value) | _R_SHIFT_ALPHA(value))
#define INVERTED_LEFT_SHIFT(value) (_INVERTED_LEFT_SHIFT(value) | _L_ALPHA)
#define INVERTED_LEFT_SHIFT_WITH_ALPHA(value) (_INVERTED_LEFT_SHIFT(value) | _L_SHIFT_ALPHA(value))

#define HROUND_GRECT(rect) hround(rect.x), hround(rect.y), hround(rect.w), hround(rect.h)
#define HROUND_GVEC2(vec2) hround(vec2.x), hround(vec2.y)

namespace april
{
#if TARGET_OS_IPHONE
	Image* _tryLoadingPVR(chstr filename);
#endif

	Image::Image()
	{
		this->data = NULL;
		this->w = 0;
		this->h = 0;
		this->format = FORMAT_INVALID;
		this->internalFormat = 0;
		this->compressedSize = 0;
	}
	
	Image::~Image()
	{
		if (this->data != NULL)
		{
			delete [] this->data;
		}
	}

	int Image::getBpp()
	{
		return Image::getFormatBpp(this->format);
	}

	int Image::getByteSize()
	{
		return (this->w * this->h * Image::getFormatBpp(this->format));
	}

	bool Image::isValid()
	{
		return (this->data != NULL && this->getByteSize() > 0);
	}

	bool Image::clear()
	{
		bool result = this->isValid();
		if (result)
		{
			memset(this->data, 0, this->getByteSize());
		}
		return (result);
	}

	Color Image::getPixel(int x, int y)
	{
		return (this->isValid() ? Image::getPixel(x, y, this->data, this->w, this->h, this->format) : Color::Clear);
	}
	
	bool Image::setPixel(int x, int y, Color color)
	{
		return (this->isValid() && Image::setPixel(x, y, color, this->data, this->w, this->h, this->format));
	}
	
	Color Image::getInterpolatedPixel(float x, float y)
	{
		return (this->isValid() ? Image::getInterpolatedPixel(x, y, this->data, this->w, this->h, this->format) : Color::Clear);
	}
	
	bool Image::fillRect(int x, int y, int w, int h, Color color)
	{
		return (this->isValid() && Image::fillRect(x, y, w, h, color, this->data, this->w, this->h, this->format));
	}

	bool Image::copyPixelData(unsigned char** output, Format format)
	{
		return (this->isValid() && Image::convertToFormat(this->w, this->h, this->data, this->format, output, format, false));
	}

	bool Image::write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		return (this->isValid() && Image::write(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, this->data, this->w, this->h, this->format));
	}

	bool Image::writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		return (this->isValid() && Image::writeStretch(sx, sy, sw, sh, dx, dy, dw, dh, srcData, srcWidth, srcHeight, srcFormat, this->data, this->w, this->h, this->format));
	}

	bool Image::blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		return (this->isValid() && Image::blit(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, this->data, this->w, this->h, this->format, alpha));
	}

	bool Image::blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		return (this->isValid() && Image::blitStretch(sx, sy, sw, sh, dx, dy, dw, dh, srcData, srcWidth, srcHeight, srcFormat, this->data, this->w, this->h, this->format, alpha));
	}

	bool Image::rotateHue(int x, int y, int w, int h, float degrees)
	{
		return (this->isValid() && Image::rotateHue(x, y, w, h, degrees, this->data, this->w, this->h, this->format));
	}

	bool Image::saturate(int x, int y, int w, int h, float factor)
	{
		return (this->isValid() && Image::saturate(x, y, w, h, factor, this->data, this->w, this->h, this->format));
	}

	bool Image::insertAlphaMap(unsigned char* srcData, Format srcFormat, unsigned char median, int ambiguity)
	{
		return (this->isValid() && Image::insertAlphaMap(this->w, this->h, srcData, srcFormat, this->data, this->format, median, ambiguity));
	}

	// overloads

	Color Image::getPixel(gvec2 position)
	{
		return this->getPixel(hround(position.x), hround(position.y));
	}
	
	bool Image::setPixel(gvec2 position, Color color)
	{
		return this->setPixel(hround(position.x), hround(position.y), color);
	}
	
	Color Image::getInterpolatedPixel(gvec2 position)
	{
		return this->getInterpolatedPixel(position.x, position.y);
	}
	
	bool Image::copyPixelData(unsigned char** output)
	{
		return (this->data != NULL && Image::convertToFormat(this->w, this->h, this->data, this->format, output, this->format, false));
	}

	bool Image::fillRect(grect rect, Color color)
	{
		return this->fillRect(HROUND_GRECT(rect), color);
	}

	bool Image::write(int sx, int sy, int sw, int sh, int dx, int dy, Image* other)
	{
		return this->write(sx, sy, sw, sh, dx, dy, other->data, other->w, other->h, other->format);
	}

	bool Image::write(grect srcRect, gvec2 destPosition, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		return this->write(HROUND_GRECT(srcRect), HROUND_GVEC2(destPosition), srcData, srcWidth, srcHeight, srcFormat);
	}

	bool Image::write(grect srcRect, gvec2 destPosition, Image* other)
	{
		return this->write(HROUND_GRECT(srcRect), HROUND_GVEC2(destPosition), other->data, other->w, other->h, other->format);
	}

	bool Image::writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Image* other)
	{
		return this->writeStretch(sx, sy, sw, sh, dx, dy, dw, dh, other->data, other->w, other->h, other->format);
	}

	bool Image::writeStretch(grect srcRect, grect destRect, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		return this->writeStretch(HROUND_GRECT(srcRect), HROUND_GRECT(destRect), srcData, srcWidth, srcHeight, srcFormat);
	}

	bool Image::writeStretch(grect srcRect, grect destRect, Image* other)
	{
		return this->writeStretch(HROUND_GRECT(srcRect), HROUND_GRECT(destRect), other->data, other->w, other->h, other->format);
	}

	bool Image::blit(int sx, int sy, int sw, int sh, int dx, int dy, Image* other, unsigned char alpha)
	{
		return this->blit(sx, sy, sw, sh, dx, dy, other->data, other->w, other->h, other->format, alpha);
	}

	bool Image::blit(grect srcRect, gvec2 destPosition, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		return this->blit(HROUND_GRECT(srcRect), HROUND_GVEC2(destPosition), srcData, srcWidth, srcHeight, srcFormat);
	}

	bool Image::blit(grect srcRect, gvec2 destPosition, Image* other, unsigned char alpha)
	{
		return this->blit(HROUND_GRECT(srcRect), HROUND_GVEC2(destPosition), other->data, other->w, other->h, other->format, alpha);
	}

	bool Image::blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Image* other, unsigned char alpha)
	{
		return this->blitStretch(sx, sy, sw, sh, dx, dy, dw, dh, other->data, other->w, other->h, other->format, alpha);
	}

	bool Image::blitStretch(grect srcRect, grect destRect, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		return this->blitStretch(HROUND_GRECT(srcRect), HROUND_GRECT(destRect), srcData, srcWidth, srcHeight, srcFormat);
	}

	bool Image::blitStretch(grect srcRect, grect destRect, Image* other, unsigned char alpha)
	{
		return this->blitStretch(HROUND_GRECT(srcRect), HROUND_GRECT(destRect), other->data, other->w, other->h, other->format, alpha);
	}

	bool Image::rotateHue(grect rect, float degrees)
	{
		return this->rotateHue(HROUND_GRECT(rect), degrees);
	}

	bool Image::saturate(grect rect, float factor)
	{
		return this->saturate(HROUND_GRECT(rect), factor);
	}

	bool Image::insertAlphaMap(Image* image, Image::Format srcFormat, unsigned char median, int ambiguity)
	{
		return this->insertAlphaMap(image->data, image->format, median, ambiguity);
	}

	// loading/creating functions

	Image* Image::createFromResource(chstr filename)
	{
		Image* image = NULL;
		if (filename.lower().ends_with(".png"))
		{
			hresource f(filename);
			image = Image::_loadPng(f);
		}
		else if (filename.lower().ends_with(".jpg") || filename.lower().ends_with(".jpeg"))
		{
			hresource f(filename);
			image = Image::_loadJpg(f);
		}
		else if (filename.lower().ends_with(".jpt"))
		{
			hresource f(filename);
			image = Image::_loadJpt(f);
		}
#if TARGET_OS_IPHONE
		else if (filename.lower().ends_with(".pvr"))
		{
			// TODOa - might need to be refactored
			image = _tryLoadingPVR(filename);
		}
#endif
		return image;
	}

	Image* Image::createFromResource(chstr filename, Image::Format format)
	{
		Image* image = Image::createFromResource(filename);
		if (image != NULL && Image::needsConversion(image->format, format))
		{
			unsigned char* data = NULL;
			if (Image::convertToFormat(image->w, image->h, image->data, image->format, &data, format))
			{
				delete [] image->data;
				image->format = format;
				image->data = data;
			}
		}
		return image;
	}

	Image* Image::createFromFile(chstr filename)
	{
		Image* image = NULL;
		if (filename.lower().ends_with(".png"))
		{
			hfile f(filename);
			image = Image::_loadPng(f);
		}
		else if (filename.lower().ends_with(".jpg") || filename.lower().ends_with(".jpeg"))
		{
			hfile f(filename);
			image = Image::_loadJpg(f);
		}
		else if (filename.lower().ends_with(".jpt"))
		{
			hfile f(filename);
			image = Image::_loadJpt(f);
		}
#if TARGET_OS_IPHONE
		else if (filename.lower().ends_with(".pvr"))
		{
			// TODOa - might need to be refactored
			image = _tryLoadingPVR(filename);
		}
#endif
		return image;
	}

	Image* Image::createFromFile(chstr filename, Image::Format format)
	{
		Image* image = Image::createFromFile(filename);
		if (image != NULL && Image::needsConversion(image->format, format))
		{
			unsigned char* data = NULL;
			if (Image::convertToFormat(image->w, image->h, image->data, image->format, &data, format))
			{
				delete [] image->data;
				image->format = format;
				image->data = data;
			}
		}
		return image;
	}

	Image* Image::create(int w, int h, unsigned char* data, Image::Format format)
	{
		Image* image = new Image();
		image->w = w;
		image->h = h;
		image->format = format;
		image->compressedSize = 0;
		int size = image->getByteSize();
		image->data = NULL;
		if (data != NULL && size > 0)
		{
			image->data = new unsigned char[size];
			memcpy(image->data, data, size);
		}
		return image;
	}

	Image* Image::create(int w, int h, Color color, Image::Format format)
	{
		Image* image = new Image();
		image->w = w;
		image->h = h;
		image->format = format;
		image->compressedSize = 0;
		int size = image->getByteSize();
		image->data = new unsigned char[size];
		image->fillRect(0, 0, image->w, image->h, color);
		return image;
	}

	Image* Image::create(Image* other)
	{
		Image* image = new Image();
		image->w = other->w;
		image->h = other->h;
		image->format = other->format;
		image->compressedSize = other->compressedSize;
		int size = image->getByteSize();
		image->data = NULL;
		if (other->data != NULL)
		{
			if (size == 0 && image->compressedSize > 0)
			{
				size = image->compressedSize;
			}
			if (size > 0)
			{
				image->data = new unsigned char[size];
				memcpy(image->data, other->data, size);
			}
		}
		return image;
	}

	int Image::getFormatBpp(Image::Format format)
	{
		switch (format)
		{
		case FORMAT_RGBA:		return 4;
		case FORMAT_ARGB:		return 4;
		case FORMAT_BGRA:		return 4;
		case FORMAT_ABGR:		return 4;
		case FORMAT_RGBX:		return 4;
		case FORMAT_XRGB:		return 4;
		case FORMAT_BGRX:		return 4;
		case FORMAT_XBGR:		return 4;
		case FORMAT_RGB:		return 3;
		case FORMAT_BGR:		return 3;
		case FORMAT_ALPHA:		return 1;
		case FORMAT_GRAYSCALE:	return 1;
		}
		return 0;
	}

	// image data manipulation functions

	Color Image::getPixel(int x, int y, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat)
	{
		Color color = Color::Clear;
		unsigned char* rgba = NULL;
		if (Image::checkRect(x, y, srcWidth, srcHeight) && Image::convertToFormat(1, 1, &srcData[(x + y * srcWidth) * Image::getFormatBpp(srcFormat)], srcFormat, &rgba, Image::FORMAT_RGBA, false))
		{
			color.r = rgba[0];
			color.g = rgba[1];
			color.b = rgba[2];
			color.a = rgba[3];
			delete [] rgba;
		}
		return color;
	}
	
	bool Image::setPixel(int x, int y, Color color, unsigned char* destData, int destWidth, int destHeight, Format destFormat)
	{
		if (!Image::checkRect(x, y, destWidth, destHeight))
		{
			return false;
		}
		unsigned char rgba[4] = {color.r, color.g, color.b, color.a};
		unsigned char* p = &destData[(x + y * destWidth) * Image::getFormatBpp(destFormat)];
		return Image::convertToFormat(1, 1, rgba, Image::FORMAT_RGBA, &p, destFormat, false);
	}

	Color Image::getInterpolatedPixel(float x, float y, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat)
	{
		Color result;
		int x0 = (int)x;
		int y0 = (int)y;
		int x1 = x0 + 1;
		int y1 = y0 + 1;
		float rx0 = x - x0;
		float ry0 = y - y0;
		float rx1 = 1.0f - rx0;
		float ry1 = 1.0f - ry0;
		if (rx0 != 0.0f && ry0 != 0.0f)
		{
			Color tl = Image::getPixel(x0, y0, srcData, srcWidth, srcHeight, srcFormat);
			Color tr = Image::getPixel(x1, y0, srcData, srcWidth, srcHeight, srcFormat);
			Color bl = Image::getPixel(x0, y1, srcData, srcWidth, srcHeight, srcFormat);
			Color br = Image::getPixel(x1, y1, srcData, srcWidth, srcHeight, srcFormat);
			result = (tl * ry1 + bl * ry0) * rx1 + (tr * ry1 + br * ry0) * rx0;
		}
		else if (rx0 != 0.0f)
		{
			Color tl = Image::getPixel(x0, y0, srcData, srcWidth, srcHeight, srcFormat);
			Color tr = Image::getPixel(x1, y0, srcData, srcWidth, srcHeight, srcFormat);
			result = tl * rx1 + tr * rx0;
		}
		else if (ry0 != 0.0f)
		{
			Color tl = Image::getPixel(x0, y0, srcData, srcWidth, srcHeight, srcFormat);
			Color bl = Image::getPixel(x0, y1, srcData, srcWidth, srcHeight, srcFormat);
			result = tl * ry1 + bl * ry0;
		}
		else
		{
			result = Image::getPixel(x0, y0, srcData, srcWidth, srcHeight, srcFormat);
		}
		return result;
	}

	bool Image::fillRect(int x, int y, int w, int h, Color color, unsigned char* destData, int destWidth, int destHeight, Format destFormat)
	{
		if (!Image::correctRect(x, y, w, h, destWidth, destHeight))
		{
			return false;
		}
		int destBpp = Image::getFormatBpp(destFormat);
		int i = (x + y * destWidth) * destBpp;
		int copyWidth = w * destBpp;
		int size = copyWidth * destHeight;
		if (destBpp == 1 || (destBpp == 3 && color.r == color.g && color.r == color.b) || (destBpp == 4 && color.r == color.g && color.r == color.b && color.r == color.a))
		{
			if (x == 0 && w == destWidth)
			{
				memset(&destData[i], color.r, copyWidth * h);
			}
			else
			{
				for_iter (j, 0, h)
				{
					memset(&destData[(x + (y + j) * destWidth) * destBpp], color.r, copyWidth);
				}
			}
			return true;
		}
		unsigned char colorData[4] = {color.r, color.g, color.b, color.a};
		// convert to right format first
		Format srcFormat = (destBpp == 4 ? FORMAT_RGBA : (destBpp == 3 ? FORMAT_RGB : FORMAT_GRAYSCALE));
		if (srcFormat != destFormat && destBpp > 1)
		{
			unsigned char* rgba = NULL;
			if (!Image::convertToFormat(1, 1, colorData, srcFormat, &rgba, destFormat))
			{
				return false;
			}
			memcpy(&destData[i], rgba, destBpp);
			delete [] rgba;
		}
		else
		{
			memcpy(&destData[i], colorData, destBpp);
		}
		int currentSize = destBpp;
		int copySize = 0;
		if (x == 0 && w == destWidth)
		{
			while (currentSize < size)
			{
				copySize = hmin(size - currentSize, currentSize);
				memcpy(&destData[i + currentSize], &destData[i], copySize);
				currentSize += copySize;
			}
		}
		else
		{
			// copy on first line
			while (currentSize < copyWidth)
			{
				copySize = hmin(copyWidth - currentSize, currentSize);
				memcpy(&destData[i + currentSize], &destData[i], copySize);
				currentSize += copySize;
			}
			// copy to all lines
			for_iter (j, 1, h)
			{
				memcpy(&destData[(x + (y + j) * destWidth) * destBpp], &destData[i], currentSize);
			}
		}
		return true;
	}
	
	bool Image::write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat,
		unsigned char* destData, int destWidth, int destHeight, Format destFormat)
	{
		if (!Image::correctRect(sx, sy, sw, sh, srcWidth, srcHeight, dx, dy, destWidth, destHeight))
		{
			return false;
		}
		int srcBpp = Image::getFormatBpp(srcFormat);
		int destBpp = Image::getFormatBpp(destFormat);
		if (srcFormat == FORMAT_ALPHA && destFormat != FORMAT_ALPHA)
		{
			if (destBpp == 4)
			{
				if (CHECK_ALPHA_FORMAT(destFormat))
				{
					int x = 0;
					int y = 0;
					int da = -1;
					Image::_getFormatIndices(destFormat, NULL, NULL, NULL, &da);
					for_iterx (y, 0, sh)
					{
						for_iterx (x, 0, sw)
						{
							destData[((dx + x) + (dy + y) * destWidth) * destBpp + da] = srcData[((sx + x) + (sy + y) * srcWidth) * srcBpp];
						}
					}
				}
				return true;
			}
			return false;
		}
		unsigned char* p = &destData[(dx + dy * destWidth) * destBpp];
		if (sx == 0 && dx == 0 && srcWidth == destWidth && sw == destWidth)
		{
			Image::convertToFormat(sw, sh, &srcData[(sx + sy * srcWidth) * srcBpp], srcFormat, &p, destFormat, false);
		}
		else
		{
			for_iter (j, 0, sh)
			{
				Image::convertToFormat(sw, 1, &srcData[(sx + (sy + j) * srcWidth) * srcBpp], srcFormat, &p, destFormat, false);
				p += destWidth * destBpp;
			}
		}
		return true;
	}

	bool Image::writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat,
		unsigned char* destData, int destWidth, int destHeight, Format destFormat)
	{
		if (!Image::correctRect(sx, sy, sw, sh, srcWidth, srcHeight, dx, dy, dw, dh, destWidth, destHeight))
		{
			return false;
		}
		if (sw == dw && sh == dh)
		{
			return Image::write(sx, sy, sw, sh, dx, dh, srcData, srcWidth, srcHeight, srcFormat, destData, destWidth, destHeight, destFormat);
		}
		int bpp = Image::getFormatBpp(destFormat);
		float fw = (float)sw / dw;
		float fh = (float)sh / dh;
		unsigned char* dest = NULL;
		unsigned char* ctl;
		unsigned char* ctr;
		unsigned char* cbl;
		unsigned char* cbr;
		float srcX;
		float srcY;
		int x0;
		int y0;
		int x1;
		int y1;
		float rx0;
		float ry0;
		float rx1;
		float ry1;
		int x = 0;
		int y = 0;
		if (srcFormat == FORMAT_ALPHA && destFormat != FORMAT_ALPHA)
		{
			if (bpp == 4)
			{
				if (CHECK_ALPHA_FORMAT(destFormat))
				{
					int da = -1;
					Image::_getFormatIndices(destFormat, NULL, NULL, NULL, &da);
					for_iterx (y, 0, dh)
					{
						srcY = sy + y * fh;
						y0 = (int)srcY;
						ry0 = srcY - y0;
						y1 = hmin(y0 + 1, srcHeight - 1);
						ry1 = 1.0f - ry0;
						for_iterx (x, 0, dw)
						{
							dest = &destData[((dx + x) + (dy + y) * destWidth) * bpp];
							srcX = sx + x * fw;
							x0 = (int)srcX;
							rx0 = srcX - x0;
							// linear interpolation
							ctl = &srcData[(x0 + y0 * srcWidth) * bpp];
							if (rx0 != 0.0f && ry0 != 0.0f)
							{
								x1 = hmin(x0 + 1, srcWidth - 1);
								rx1 = 1.0f - rx0;
								ctr = &srcData[(x1 + y0 * srcWidth) * bpp];
								cbl = &srcData[(x0 + y1 * srcWidth) * bpp];
								cbr = &srcData[(x1 + y1 * srcWidth) * bpp];
								dest[da] = (unsigned char)(((ctl[0] * ry1 + cbl[0] * ry0) * rx1 + (ctr[0] * ry1 + cbr[0] * ry0) * rx0));
							}
							else if (rx0 != 0.0f)
							{
								x1 = hmin(x0 + 1, srcWidth - 1);
								rx1 = 1.0f - rx0;
								ctr = &srcData[(x1 + y0 * srcWidth) * bpp];
								dest[da] = (unsigned char)((ctl[0] * rx1 + ctr[0] * rx0));
							}
							else if (ry0 != 0.0f)
							{
								cbl = &srcData[(x0 + y1 * srcWidth) * bpp];
								dest[da] = (unsigned char)((ctl[0] * ry1 + cbl[0] * ry0));
							}
							else
							{
								dest[da] = ctl[0];
							}
						}
					}
				}
				return true;
			}
			return false;
		}
		bool createNew = Image::needsConversion(srcFormat, destFormat);
		if (createNew)
		{
			unsigned char* data = srcData;
			srcData = new unsigned char[sw * sh * bpp];
			if (!Image::write(sx, sy, sw, sh, 0, 0, data, srcWidth, srcHeight, srcFormat, srcData, sw, sh, destFormat))
			{
				delete [] srcData;
				return false;
			}
			// changed size of data, needs to readjust
			sx = 0;
			sy = 0;
			srcWidth = sw;
			srcHeight = sh;
		}
		bool result = false;
		if (bpp == 1)
		{
			for_iterx (y, 0, dh)
			{
				srcY = sy + y * fh;
				y0 = (int)srcY;
				ry0 = srcY - y0;
				y1 = hmin(y0 + 1, srcHeight - 1);
				ry1 = 1.0f - ry0;
				for_iterx (x, 0, dw)
				{
					dest = &destData[((dx + x) + (dy + y) * destWidth) * bpp];
					srcX = sx + x * fw;
					x0 = (int)srcX;
					rx0 = srcX - x0;
					// linear interpolation
					ctl = &srcData[(x0 + y0 * srcWidth) * bpp];
					if (rx0 != 0.0f && ry0 != 0.0f)
					{
						x1 = hmin(x0 + 1, srcWidth - 1);
						rx1 = 1.0f - rx0;
						ctr = &srcData[(x1 + y0 * srcWidth) * bpp];
						cbl = &srcData[(x0 + y1 * srcWidth) * bpp];
						cbr = &srcData[(x1 + y1 * srcWidth) * bpp];
						dest[0] = (unsigned char)(((ctl[0] * ry1 + cbl[0] * ry0) * rx1 + (ctr[0] * ry1 + cbr[0] * ry0) * rx0));
					}
					else if (rx0 != 0.0f)
					{
						x1 = hmin(x0 + 1, srcWidth - 1);
						rx1 = 1.0f - rx0;
						ctr = &srcData[(x1 + y0 * srcWidth) * bpp];
						dest[0] = (unsigned char)((ctl[0] * rx1 + ctr[0] * rx0));
					}
					else if (ry0 != 0.0f)
					{
						cbl = &srcData[(x0 + y1 * srcWidth) * bpp];
						dest[0] = (unsigned char)((ctl[0] * ry1 + cbl[0] * ry0));
					}
					else
					{
						dest[0] = ctl[0];
					}
				}
			}
			result = true;
		}
		else if (bpp == 3)
		{
			for_iterx (y, 0, dh)
			{
				srcY = sy + y * fh;
				y0 = (int)srcY;
				ry0 = srcY - y0;
				y1 = hmin(y0 + 1, srcHeight - 1);
				ry1 = 1.0f - ry0;
				for_iterx (x, 0, dw)
				{
					dest = &destData[((dx + x) + (dy + y) * destWidth) * bpp];
					srcX = sx + x * fw;
					x0 = (int)srcX;
					rx0 = srcX - x0;
					// linear interpolation
					ctl = &srcData[(x0 + y0 * srcWidth) * bpp];
					if (rx0 != 0.0f && ry0 != 0.0f)
					{
						x1 = hmin(x0 + 1, srcWidth - 1);
						rx1 = 1.0f - rx0;
						ctr = &srcData[(x1 + y0 * srcWidth) * bpp];
						cbl = &srcData[(x0 + y1 * srcWidth) * bpp];
						cbr = &srcData[(x1 + y1 * srcWidth) * bpp];
						dest[0] = (unsigned char)(((ctl[0] * ry1 + cbl[0] * ry0) * rx1 + (ctr[0] * ry1 + cbr[0] * ry0) * rx0));
						dest[1] = (unsigned char)(((ctl[1] * ry1 + cbl[1] * ry0) * rx1 + (ctr[1] * ry1 + cbr[1] * ry0) * rx0));
						dest[2] = (unsigned char)(((ctl[2] * ry1 + cbl[2] * ry0) * rx1 + (ctr[2] * ry1 + cbr[2] * ry0) * rx0));
					}
					else if (rx0 != 0.0f)
					{
						x1 = hmin(x0 + 1, srcWidth - 1);
						rx1 = 1.0f - rx0;
						ctr = &srcData[(x1 + y0 * srcWidth) * bpp];
						dest[0] = (unsigned char)((ctl[0] * rx1 + ctr[0] * rx0));
						dest[1] = (unsigned char)((ctl[1] * rx1 + ctr[1] * rx0));
						dest[2] = (unsigned char)((ctl[2] * rx1 + ctr[2] * rx0));
					}
					else if (ry0 != 0.0f)
					{
						cbl = &srcData[(x0 + y1 * srcWidth) * bpp];
						dest[0] = (unsigned char)((ctl[0] * ry1 + cbl[0] * ry0));
						dest[1] = (unsigned char)((ctl[1] * ry1 + cbl[1] * ry0));
						dest[2] = (unsigned char)((ctl[2] * ry1 + cbl[2] * ry0));
					}
					else
					{
						dest[0] = ctl[0];
						dest[1] = ctl[1];
						dest[2] = ctl[2];
					}
				}
			}
			result = true;
		}
		else if (bpp == 4)
		{
			for_iterx (y, 0, dh)
			{
				srcY = sy + y * fh;
				y0 = (int)srcY;
				ry0 = srcY - y0;
				y1 = hmin(y0 + 1, srcHeight - 1);
				ry1 = 1.0f - ry0;
				for_iterx (x, 0, dw)
				{
					dest = &destData[((dx + x) + (dy + y) * destWidth) * bpp];
					srcX = sx + x * fw;
					x0 = (int)srcX;
					rx0 = srcX - x0;
					// linear interpolation
					ctl = &srcData[(x0 + y0 * srcWidth) * bpp];
					if (rx0 != 0.0f && ry0 != 0.0f)
					{
						x1 = hmin(x0 + 1, srcWidth - 1);
						rx1 = 1.0f - rx0;
						ctr = &srcData[(x1 + y0 * srcWidth) * bpp];
						cbl = &srcData[(x0 + y1 * srcWidth) * bpp];
						cbr = &srcData[(x1 + y1 * srcWidth) * bpp];
						dest[0] = (unsigned char)(((ctl[0] * ry1 + cbl[0] * ry0) * rx1 + (ctr[0] * ry1 + cbr[0] * ry0) * rx0));
						dest[1] = (unsigned char)(((ctl[1] * ry1 + cbl[1] * ry0) * rx1 + (ctr[1] * ry1 + cbr[1] * ry0) * rx0));
						dest[2] = (unsigned char)(((ctl[2] * ry1 + cbl[2] * ry0) * rx1 + (ctr[2] * ry1 + cbr[2] * ry0) * rx0));
						dest[3] = (unsigned char)(((ctl[3] * ry1 + cbl[3] * ry0) * rx1 + (ctr[3] * ry1 + cbr[3] * ry0) * rx0));
					}
					else if (rx0 != 0.0f)
					{
						x1 = hmin(x0 + 1, srcWidth - 1);
						rx1 = 1.0f - rx0;
						ctr = &srcData[(x1 + y0 * srcWidth) * bpp];
						dest[0] = (unsigned char)((ctl[0] * rx1 + ctr[0] * rx0));
						dest[1] = (unsigned char)((ctl[1] * rx1 + ctr[1] * rx0));
						dest[2] = (unsigned char)((ctl[2] * rx1 + ctr[2] * rx0));
						dest[3] = (unsigned char)((ctl[3] * rx1 + ctr[3] * rx0));
					}
					else if (ry0 != 0.0f)
					{
						cbl = &srcData[(x0 + y1 * srcWidth) * bpp];
						dest[0] = (unsigned char)((ctl[0] * ry1 + cbl[0] * ry0));
						dest[1] = (unsigned char)((ctl[1] * ry1 + cbl[1] * ry0));
						dest[2] = (unsigned char)((ctl[2] * ry1 + cbl[2] * ry0));
						dest[3] = (unsigned char)((ctl[3] * ry1 + cbl[3] * ry0));
					}
					else
					{
						dest[0] = ctl[0];
						dest[1] = ctl[1];
						dest[2] = ctl[2];
						dest[3] = ctl[3];
					}
				}
			}
			result = true;
		}
		if (createNew)
		{
			delete [] srcData;
		}
		return result;
	}

	bool Image::blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat,
		unsigned char* destData, int destWidth, int destHeight, Format destFormat, unsigned char alpha)
	{
		if (!Image::correctRect(sx, sy, sw, sh, srcWidth, srcHeight, dx, dy, destWidth, destHeight))
		{
			return false;
		}
		// source format doesn't have alpha and no alpha multiplier is used, so using write() is enough
		if (!CHECK_ALPHA_FORMAT(srcFormat) && alpha == 255)
		{
			return Image::write(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, destData, destWidth, destHeight, destFormat);
		}
		// it's invisible anyway, so let's say it's successful
		if (alpha == 0)
		{
			return true;
		}
		int srcBpp = Image::getFormatBpp(srcFormat);
		if (srcBpp == 1)
		{
			if (Image::_blitFrom1Bpp(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, destData, destWidth, destHeight, destFormat, alpha))
			{
				return true;
			}
		}
		else if (srcBpp == 3)
		{
			if (Image::_blitFrom3Bpp(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, destData, destWidth, destHeight, destFormat, alpha))
			{
				return true;
			}
		}
		else if (srcBpp == 4)
		{
			if (Image::_blitFrom4Bpp(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, destData, destWidth, destHeight, destFormat, alpha))
			{
				return true;
			}
		}
		return false;
	}

	bool Image::_blitFrom1Bpp(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Format destFormat, unsigned char alpha)
	{
		static int srcBpp = 1;
		int destBpp = Image::getFormatBpp(destFormat);
		if (srcFormat == FORMAT_ALPHA && destFormat != FORMAT_ALPHA && destBpp != 4)
		{
			return false;
		}
		unsigned char* src = NULL;
		unsigned char* dest = NULL;
		unsigned char a1;
		unsigned int c;
		int x = 0;
		int y = 0;
		a1 = 255 - alpha;
		if (destBpp == 1)
		{
			for_iterx (y, 0, sh)
			{
				for_iterx (x, 0, sw)
				{
					src = &srcData[((sx + x) + (sy + y) * srcWidth) * srcBpp];
					dest = &destData[((dx + x) + (dy + y) * destWidth) * destBpp];
					dest[0] = (src[0] * alpha + dest[0] * a1) / 255;
				}
			}
			return true;
		}
		int dr = -1;
		int dg = -1;
		int db = -1;
		if (destBpp == 3 || !CHECK_ALPHA_FORMAT(destFormat)) // 3 BPP and 4 BPP without alpha
		{
			if (srcFormat != FORMAT_ALPHA)
			{
				Image::_getFormatIndices(destFormat, &dr, &dg, &db, NULL);
				for_iterx (y, 0, sh)
				{
					for_iterx (x, 0, sw)
					{
						src = &srcData[((sx + x) + (sy + y) * srcWidth) * srcBpp];
						dest = &destData[((dx + x) + (dy + y) * destWidth) * destBpp];
						c = src[0] * alpha;
						dest[dr] = (c + dest[dr] * a1) / 255;
						dest[dg] = (c + dest[dg] * a1) / 255;
						dest[db] = (c + dest[db] * a1) / 255;
					}
				}
			}
			return true;
		}
		int da = -1;
		if (destBpp == 4) // 4 BPP with alpha
		{
			Image::_getFormatIndices(destFormat, &dr, &dg, &db, &da);
			if (srcFormat != FORMAT_ALPHA)
			{
				for_iterx (y, 0, sh)
				{
					for_iterx (x, 0, sw)
					{
						src = &srcData[((sx + x) + (sy + y) * srcWidth) * srcBpp];
						dest = &destData[((dx + x) + (dy + y) * destWidth) * destBpp];
						c = src[0] * alpha;
						dest[dr] = (c + dest[dr] * a1) / 255;
						dest[dg] = (c + dest[dg] * a1) / 255;
						dest[db] = (c + dest[db] * a1) / 255;
						dest[da] = alpha + dest[da] * a1 / 255;
					}
				}
			}
			else
			{
				for_iterx (y, 0, sh)
				{
					for_iterx (x, 0, sw)
					{
						src = &srcData[((sx + x) + (sy + y) * srcWidth) * srcBpp];
						dest = &destData[((dx + x) + (dy + y) * destWidth) * destBpp];
						dest[da] = (src[0] * alpha + dest[da] * a1) / 255;
					}
				}
			}
			return true;
		}
		return false;
	}

	bool Image::_blitFrom3Bpp(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Format destFormat, unsigned char alpha)
	{
		static int srcBpp = 3;
		int destBpp = Image::getFormatBpp(destFormat);
		unsigned char* src = NULL;
		unsigned char* dest = NULL;
		unsigned char a1;
		int x = 0;
		int y = 0;
		a1 = 255 - alpha;
		int sr = -1;
		if (destBpp == 1)
		{
			Image::_getFormatIndices(srcFormat, &sr, NULL, NULL, NULL);
			for_iterx (y, 0, sh)
			{
				for_iterx (x, 0, sw)
				{
					src = &srcData[((sx + x) + (sy + y) * srcWidth) * srcBpp];
					dest = &destData[((dx + x) + (dy + y) * destWidth) * destBpp];
					dest[0] = (src[sr] * alpha + dest[0] * a1) / 255;
				}
			}
			return true;
		}
		int sg = -1;
		int sb = -1;
		Image::_getFormatIndices(srcFormat, &sr, &sg, &sb, NULL);
		int dr = -1;
		int dg = -1;
		int db = -1;
		if (destBpp == 3 || !CHECK_ALPHA_FORMAT(destFormat)) // 3 BPP and 4 BPP without alpha
		{
			Image::_getFormatIndices(destFormat, &dr, &dg, &db, NULL);
			for_iterx (y, 0, sh)
			{
				for_iterx (x, 0, sw)
				{
					src = &srcData[((sx + x) + (sy + y) * srcWidth) * srcBpp];
					dest = &destData[((dx + x) + (dy + y) * destWidth) * destBpp];
					dest[dr] = (src[sr] * alpha + dest[dr] * a1) / 255;
					dest[dg] = (src[sg] * alpha + dest[dg] * a1) / 255;
					dest[db] = (src[sb] * alpha + dest[db] * a1) / 255;
				}
			}
			return true;
		}
		int da = -1;
		if (destBpp == 4) // 4 BPP with alpha
		{
			Image::_getFormatIndices(destFormat, &dr, &dg, &db, &da);
			for_iterx (y, 0, sh)
			{
				for_iterx (x, 0, sw)
				{
					src = &srcData[((sx + x) + (sy + y) * srcWidth) * srcBpp];
					dest = &destData[((dx + x) + (dy + y) * destWidth) * destBpp];
					dest[dr] = (src[sr] * alpha + dest[dr] * a1) / 255;
					dest[dg] = (src[sg] * alpha + dest[dg] * a1) / 255;
					dest[db] = (src[sb] * alpha + dest[db] * a1) / 255;
					dest[da] = alpha + dest[da] * a1 / 255;
				}
			}
			return true;
		}
		return false;
	}
	
	bool Image::_blitFrom4Bpp(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Format destFormat, unsigned char alpha)
	{
		static int srcBpp = 4;
		int destBpp = Image::getFormatBpp(destFormat);
		unsigned char* src = NULL;
		unsigned char* dest = NULL;
		unsigned char a0;
		unsigned char a1;
		int x = 0;
		int y = 0;
		int sr = -1;
		int sa = -1;
		if (destBpp == 1)
		{
			Image::_getFormatIndices(srcFormat, &sr, NULL, NULL, &sa);
			for_iterx (y, 0, sh)
			{
				for_iterx (x, 0, sw)
				{
					src = &srcData[((sx + x) + (sy + y) * srcWidth) * srcBpp];
					dest = &destData[((dx + x) + (dy + y) * destWidth) * destBpp];
					a0 = src[sa] * alpha / 255;
					if (a0 > 0)
					{
						dest[0] = (src[sr] * a0 + dest[0] * (255 - a0)) / 255;
					}
				}
			}
			return true;
		}
		int sg = -1;
		int sb = -1;
		Image::_getFormatIndices(srcFormat, &sr, &sg, &sb, &sa);
		int dr = -1;
		int dg = -1;
		int db = -1;
		if (destBpp == 3 || !CHECK_ALPHA_FORMAT(destFormat)) // 3 BPP and 4 BPP without alpha
		{
			Image::_getFormatIndices(destFormat, &dr, &dg, &db, NULL);
			for_iterx (y, 0, sh)
			{
				for_iterx (x, 0, sw)
				{
					src = &srcData[((sx + x) + (sy + y) * srcWidth) * srcBpp];
					dest = &destData[((dx + x) + (dy + y) * destWidth) * destBpp];
					a0 = src[sa] * alpha / 255;
					if (a0 > 0)
					{
						a1 = 255 - a0;
						dest[dr] = (src[sr] * a0 + dest[dr] * a1) / 255;
						dest[dg] = (src[sg] * a0 + dest[dg] * a1) / 255;
						dest[db] = (src[sb] * a0 + dest[db] * a1) / 255;
					}
				}
			}
			return true;
		}
		int da = -1;
		if (destBpp == 4) // 4 BPP with alpha
		{
			Image::_getFormatIndices(destFormat, &dr, &dg, &db, &da);
			for_iterx (y, 0, sh)
			{
				for_iterx (x, 0, sw)
				{
					src = &srcData[((sx + x) + (sy + y) * srcWidth) * srcBpp];
					dest = &destData[((dx + x) + (dy + y) * destWidth) * destBpp];
					a0 = src[sa] * alpha / 255;
					if (a0 > 0)
					{
						a1 = (255 - a0);
						dest[dr] = (src[sr] * a0 + dest[dr] * a1) / 255;
						dest[dg] = (src[sg] * a0 + dest[dg] * a1) / 255;
						dest[db] = (src[sb] * a0 + dest[db] * a1) / 255;
						dest[da] = a0 + dest[da] * a1 / 255;
					}
				}
			}
			return true;
		}
		return false;
	}

	bool Image::blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat,
		unsigned char* destData, int destWidth, int destHeight, Format destFormat, unsigned char alpha)
	{
		if (!Image::correctRect(sx, sy, sw, sh, srcWidth, srcHeight, dx, dy, dw, dh, destWidth, destHeight))
		{
			return false;
		}
		if (sw == dw && sh == dh)
		{
			return Image::blit(sx, sy, sw, sh, dx, dh, srcData, srcWidth, srcHeight, srcFormat, destData, destWidth, destHeight, destFormat, alpha);
		}
		unsigned char* stretched = new unsigned char[dw * dh * Image::getFormatBpp(srcFormat)];
		bool result = Image::writeStretch(sx, sy, sw, sh, 0, 0, dw, dh, srcData, srcWidth, srcHeight, srcFormat, stretched, dw, dh, srcFormat);
		if (result)
		{
			result = Image::blit(0, 0, dw, dh, dx, dy, stretched, dw, dh, srcFormat, destData, destWidth, destHeight, destFormat, alpha);
		}
		delete [] stretched;
		return result;
	}

	bool Image::rotateHue(int x, int y, int w, int h, float degrees, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		if (!Image::correctRect(x, y, w, h, srcWidth, srcHeight))
		{
			return false;
		}
		int srcBpp = Image::getFormatBpp(srcFormat);
		if (srcBpp == 1)
		{
			return true;
		}
		float range = hmodf(degrees / 360.0f, 1.0f);
		if (range == 0.0f)
		{
			return true;
		}
		int sr = -1;
		int sg = -1;
		int sb = -1;
		Image::_getFormatIndices(srcFormat, &sr, &sg, &sb, NULL);
		float _h;
		float _s;
		float _l;
		int i;
		for_iter (dy, 0, h)
		{
			for_iter (dx, 0, w)
			{
				i = ((x + dx) + (y + dy) * srcWidth) * srcBpp;
				april::rgbToHsl(srcData[i + sr], srcData[i + sg], srcData[i + sb], &_h, &_s, &_l);
				april::hslToRgb(hmodf(_h + range, 1.0f), _s, _l, &srcData[i + sr], &srcData[i + sg], &srcData[i + sb]);
			}
		}
		return true;
	}

	bool Image::saturate(int x, int y, int w, int h, float factor, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		if (!Image::correctRect(x, y, w, h, srcWidth, srcHeight))
		{
			return false;
		}
		int srcBpp = Image::getFormatBpp(srcFormat);
		if (srcBpp == 1)
		{
			return true;
		}
		int sr = -1;
		int sg = -1;
		int sb = -1;
		Image::_getFormatIndices(srcFormat, &sr, &sg, &sb, NULL);
		float _h;
		float _s;
		float _l;
		int i;
		for_iter (dy, 0, h)
		{
			for_iter (dx, 0, w)
			{
				i = ((x + dx) + (y + dy) * srcWidth) * srcBpp;
				april::rgbToHsl(srcData[i + sr], srcData[i + sg], srcData[i + sb], &_h, &_s, &_l);
				april::hslToRgb(_h, hclamp(_s * factor, 0.0f, 1.0f), _l, &srcData[i + sr], &srcData[i + sg], &srcData[i + sb]);
			}
		}
		return true;
	}

	bool Image::invert(int x, int y, int w, int h, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		if (!Image::correctRect(x, y, w, h, srcWidth, srcHeight))
		{
			return false;
		}
		int i;
		int srcBpp = Image::getFormatBpp(srcFormat);
		if (srcBpp == 1)
		{
			for_iter (dy, 0, h)
			{
				for_iter (dx, 0, w)
				{
					i = ((x + dx) + (y + dy) * srcWidth);
					srcData[i] = 255 - srcData[i];
				}
			}
			return true;
		}
		int sr = -1;
		int sg = -1;
		int sb = -1;
		Image::_getFormatIndices(srcFormat, &sr, &sg, &sb, NULL);
		for_iter (dy, 0, h)
		{
			for_iter (dx, 0, w)
			{
				i = ((x + dx) + (y + dy) * srcWidth) * srcBpp;
				srcData[i + sr] = 255 - srcData[i + sr];
				srcData[i + sg] = 255 - srcData[i + sg];
				srcData[i + sb] = 255 - srcData[i + sb];
			}
		}
		return true;
	}

	bool Image::insertAlphaMap(int w, int h, unsigned char* srcData, Image::Format srcFormat, unsigned char* destData, Image::Format destFormat, unsigned char median, int ambiguity)
	{
		if (!CHECK_ALPHA_FORMAT(destFormat)) // not a format that supports an alpha channel
		{
			return false;
		}
		int srcBpp = Image::getFormatBpp(srcFormat);
		if (srcBpp == 1 || srcBpp == 3 || srcBpp == 4)
		{
			int destBpp = Image::getFormatBpp(destFormat);
			int sr = -1;
			Image::_getFormatIndices(srcFormat, &sr, NULL, NULL, NULL);
			int da = -1;
			Image::_getFormatIndices(destFormat, NULL, NULL, NULL, &da);
			unsigned char* src = NULL;
			unsigned char* dest = NULL;
			int i = 0;
			int x = 0;
			int y = 0;
			if (ambiguity == 0)
			{
				for_iterx (y, 0, h)
				{
					for_iterx (x, 0, w)
					{
						i = (x + y * w);
						// takes the red second color channel for alpha value
						destData[i * destBpp + da] = srcData[i * srcBpp + sr];
					}
				}
			}
			else
			{
				int min = (int)median - ambiguity / 2;
				int max = (int)median + ambiguity / 2;
				for_iterx (y, 0, h)
				{
					for_iterx (x, 0, w)
					{
						i = (x + y * w);
						src = &srcData[i * srcBpp];
						dest = &destData[i * destBpp];
						// takes the red second color channel for alpha value
						if (src[sr] < min)
						{
							dest[da] = 255;
						}
						else if (src[sr] >= max)
						{
							dest[da] = 0;
						}
						else
						{
							dest[da] = (max - src[sr]) * 255 / ambiguity;
						}
					}
				}
			}
			return true;
		}
		return false;
	}

	bool Image::convertToFormat(int w, int h, unsigned char* srcData, Image::Format srcFormat, unsigned char** destData, Image::Format destFormat, bool preventCopy)
	{
		if (preventCopy && srcFormat == destFormat)
		{
			hlog::warn(april::logTag, "The source's and destination's formats are the same!");
			return false;
		}
		int srcBpp = Image::getFormatBpp(srcFormat);
		if (srcFormat == FORMAT_PALETTE && destFormat == FORMAT_PALETTE)
		{
			return true;
		}
		if (srcBpp == 1)
		{
			if (Image::_convertFrom1Bpp(w, h, srcData, srcFormat, destData, destFormat))
			{
				return true;
			}
		}
		else if (srcBpp == 3)
		{
			if (Image::_convertFrom3Bpp(w, h, srcData, srcFormat, destData, destFormat))
			{
				return true;
			}
		}
		else if (srcBpp == 4)
		{
			if (Image::_convertFrom4Bpp(w, h, srcData, srcFormat, destData, destFormat))
			{
				return true;
			}
		}
		hlog::errorf(april::logTag, "Conversion from %d BPP to %d BPP is not supported!", srcBpp, Image::getFormatBpp(destFormat));
		return false;
	}

	bool Image::_convertFrom1Bpp(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char** destData, Format destFormat)
	{
		int destBpp = Image::getFormatBpp(destFormat);
		bool createData = (*destData == NULL);
		if (createData)
		{
			*destData = new unsigned char[w * h * destBpp];
		}
		if (destBpp == 1)
		{
			memcpy(*destData, srcData, w * h * destBpp);
			return true;
		}
		int x = 0;
		int y = 0;
		if (destBpp == 3 || destBpp == 4)
		{
			int i = 0;
			if (destBpp > 3)
			{
				memset(*destData, 255, w * h * destBpp);
				if (!CHECK_LEFT_RGB(destFormat))
				{
					for_iterx (y, 0, h)
					{
						for_iterx (x, 0, w)
						{
							i = (x + y * w) * destBpp;
							(*destData)[i + 1] = (*destData)[i + 2] = (*destData)[i + 3] = srcData[x + y * w];
						}
					}
					return true;
				}
			}
			for_iterx (y, 0, h)
			{
				for_iterx (x, 0, w)
				{
					i = (x + y * w) * destBpp;
					(*destData)[i] = (*destData)[i + 1] = (*destData)[i + 2] = srcData[x + y * w];
				}
			}
			return true;
		}
		if (createData)
		{
			delete [] *destData;
			*destData = NULL;
		}
		return false;
	}

	bool Image::_convertFrom3Bpp(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char** destData, Format destFormat)
	{
		static int srcBpp = 3;
		int destBpp = Image::getFormatBpp(destFormat);
		bool createData = (*destData == NULL);
		if (createData)
		{
			*destData = new unsigned char[w * h * destBpp];
		}
		int x = 0;
		int y = 0;
		if (destBpp == 1)
		{
			int redIndex = (srcFormat == FORMAT_RGB ? 0 : 2);
			for_iterx (y, 0, h)
			{
				for_iterx (x, 0, w)
				{
					// red is used as main component
					(*destData)[x + y * w] = srcData[(x + y * w) * srcBpp + redIndex];
				}
			}
			return true;
		}
		if (destBpp == 3)
		{
			memcpy(*destData, srcData, w * h * destBpp);
			// FORMAT_RGB to FORMAT_BGR and vice versa, thus switching 2 bytes around is enough
			if (srcFormat != destFormat)
			{
				int i = 0;
				for_iterx (y, 0, h)
				{
					for_iterx (x, 0, w)
					{
						i = (x + y * w) * destBpp;
						(*destData)[i] = srcData[i + 2];
						(*destData)[i + 2] = srcData[i];
					}
				}
			}
			return true;
		}
		if (destBpp == 4)
		{
			unsigned int* dest = (unsigned int*)*destData;
			Format extended = (srcFormat == FORMAT_RGB ? FORMAT_RGBX : FORMAT_BGRX);
			bool rightShift = CHECK_SHIFT_FORMATS(extended, destFormat);
			bool invertOrder = (CHECK_INVERT_ORDER_FORMATS(extended, destFormat) || CHECK_INVERT_ORDER_FORMATS(destFormat, extended));
			int i = 0;
			if (rightShift)
			{
				if (invertOrder)
				{
					FOR_EACH_3BPP_TO_4BPP_PIXEL((((unsigned int)srcData[i]) << 24) | (((unsigned int)srcData[i + 1]) << 16) | (((unsigned int)srcData[i + 2]) << 8) | _R_ALPHA);
				}
				else
				{
					FOR_EACH_3BPP_TO_4BPP_PIXEL((((unsigned int)srcData[i]) << 8) | (((unsigned int)srcData[i + 1]) << 16) | (((unsigned int)srcData[i + 2]) << 24) | _R_ALPHA);
				}
			}
			else if (invertOrder)
			{
				FOR_EACH_3BPP_TO_4BPP_PIXEL((((unsigned int)srcData[i]) << 16) | (((unsigned int)srcData[i + 1]) << 8) | srcData[i + 2] | _L_ALPHA);
			}
			else
			{
				FOR_EACH_3BPP_TO_4BPP_PIXEL(srcData[i] | (((unsigned int)srcData[i + 1]) << 8) | (((unsigned int)srcData[i + 2]) << 16) | _L_ALPHA);
			}
			return true;
		}
		if (createData)
		{
			delete [] *destData;
			*destData = NULL;
		}
		return false;
	}

	bool Image::_convertFrom4Bpp(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char** destData, Format destFormat)
	{
		static int srcBpp = 4;
		int destBpp = Image::getFormatBpp(destFormat);
		bool createData = (*destData == NULL);
		if (createData)
		{
			*destData = new unsigned char[w * h * destBpp];
		}
		int x = 0;
		int y = 0;
		if (destBpp == 1)
		{
			int redIndex = 0;
			if (srcFormat == FORMAT_ARGB || srcFormat == FORMAT_XRGB)
			{
				redIndex = 1;
			}
			else if (srcFormat == FORMAT_BGRA || srcFormat == FORMAT_BGRX)
			{
				redIndex = 2;
			}
			else if (srcFormat == FORMAT_ABGR || srcFormat == FORMAT_XBGR)
			{
				redIndex = 3;
			}
			for_iterx (y, 0, h)
			{
				for_iterx (x, 0, w)
				{
					// red is used as main component
					(*destData)[x + y * w] = srcData[(x + y * w) * srcBpp + redIndex];
				}
			}
			return true;
		}
		if (destBpp == 3)
		{
			unsigned int* src = (unsigned int*)srcData;
			unsigned char* dest = *destData;
			Format extended = (destFormat == FORMAT_RGB ? FORMAT_RGBX : FORMAT_BGRX);
			bool leftShift = CHECK_SHIFT_FORMATS(extended, srcFormat);
			bool invertOrder = (CHECK_INVERT_ORDER_FORMATS(srcFormat, extended) || CHECK_INVERT_ORDER_FORMATS(extended, srcFormat));
			int i = 0;
			int j = 0;
			if (leftShift)
			{
				if (invertOrder)
				{
					FOR_EACH_4BPP_TO_3BPP_PIXEL(src[i] >> 24, src[i] >> 16, src[i] >> 8);
				}
				else
				{
					FOR_EACH_4BPP_TO_3BPP_PIXEL(src[i] >> 8, src[i] >> 16, src[i] >> 24);
				}
			}
			else if (invertOrder)
			{
				FOR_EACH_4BPP_TO_3BPP_PIXEL(src[i] >> 16, src[i] >> 8, src[i]);
			}
			else
			{
				FOR_EACH_4BPP_TO_3BPP_PIXEL(src[i], src[i] >> 8, src[i] >> 16);
			}
			return true;
		}
		if (destBpp == 4)
		{
			// shifting unsigned int's around is faster than pure assigning (like at 3 BPP)
			unsigned int* src = (unsigned int*)srcData;
			unsigned int* dest = (unsigned int*)*destData;
			bool rightShift = CHECK_SHIFT_FORMATS(srcFormat, destFormat);
			bool leftShift = CHECK_SHIFT_FORMATS(destFormat, srcFormat);
			bool invertOrder = (CHECK_INVERT_ORDER_FORMATS(srcFormat, destFormat) || CHECK_INVERT_ORDER_FORMATS(destFormat, srcFormat));
			bool left = CHECK_LEFT_RGB(destFormat);
			bool invertOrderL = (invertOrder && left);
			bool invertOrderR = (invertOrder && !left);
			bool srcAlpha = CHECK_ALPHA_FORMAT(srcFormat);
			bool destAlpha = CHECK_ALPHA_FORMAT(destFormat);
			bool copyAlpha = (srcAlpha && destAlpha);
			int i = 0;
			if (rightShift)
			{
				if (invertOrder)
				{
					if (copyAlpha)
					{
						FOR_EACH_4BPP_PIXEL(INVERTED_RIGHT_SHIFT_WITH_ALPHA);
					}
					else
					{
						FOR_EACH_4BPP_PIXEL(INVERTED_RIGHT_SHIFT);
					}
				}
				else if (copyAlpha)
				{
					FOR_EACH_4BPP_PIXEL(RIGHT_SHIFT_WITH_ALPHA);
				}
				else
				{
					FOR_EACH_4BPP_PIXEL(RIGHT_SHIFT);
				}
			}
			else if (leftShift)
			{
				if (invertOrder)
				{
					if (copyAlpha)
					{
						FOR_EACH_4BPP_PIXEL(INVERTED_LEFT_SHIFT_WITH_ALPHA);
					}
					else
					{
						FOR_EACH_4BPP_PIXEL(INVERTED_LEFT_SHIFT);
					}
				}
				else if (copyAlpha)
				{
					FOR_EACH_4BPP_PIXEL(LEFT_SHIFT_WITH_ALPHA);
				}
				else
				{
					FOR_EACH_4BPP_PIXEL(LEFT_SHIFT);
				}
			}
			else if (invertOrderL)
			{
				if (copyAlpha)
				{
					FOR_EACH_4BPP_PIXEL(INVERTED_L_WITH_ALPHA);
				}
				else
				{
					FOR_EACH_4BPP_PIXEL(INVERTED_L);
				}
			}
			else if (invertOrderR)
			{
				if (copyAlpha)
				{
					FOR_EACH_4BPP_PIXEL(INVERTED_R_WITH_ALPHA);
				}
				else
				{
					FOR_EACH_4BPP_PIXEL(INVERTED_R);
				}
			}
			else if (srcAlpha ^ destAlpha)
			{
				if (left)
				{
					FOR_EACH_4BPP_PIXEL(KEEP_L);
				}
				else
				{
					FOR_EACH_4BPP_PIXEL(KEEP_R);
				}
			}
			else
			{
				memcpy(*destData, srcData, w * h * destBpp);
			}
			return true;
		}
		if (createData)
		{
			delete [] *destData;
			*destData = NULL;
		}
		return false;
	}

	bool Image::needsConversion(Format srcFormat, Format destFormat, bool preventCopy)
	{
		if (preventCopy && srcFormat == destFormat)
		{
			return false;
		}
		int srcBpp = Image::getFormatBpp(srcFormat);
		int destBpp = Image::getFormatBpp(destFormat);
		if (srcBpp != destBpp)
		{
			return true;
		}
		if (srcBpp != 4)
		{
			return false;
		}
		if (CHECK_SHIFT_FORMATS(srcFormat, destFormat))
		{
			return true;
		}
		if (CHECK_SHIFT_FORMATS(destFormat, srcFormat))
		{
			return true;
		}
		if (CHECK_INVERT_ORDER_FORMATS(srcFormat, destFormat) || CHECK_INVERT_ORDER_FORMATS(destFormat, srcFormat))
		{
			return true;
		}
		if (CHECK_ALPHA_FORMAT(destFormat))
		{
			return true;
		}
		return false;
	}

	bool Image::checkRect(int dx, int dy, int destWidth, int destHeight)
	{
		return (dx >= 0 && dx < destWidth && dy >= 0 && dy < destHeight);
	}

	bool Image::checkRect(int dx, int dy, int dw, int dh, int destWidth, int destHeight)
	{
		return (Image::checkRect(dx, dy, destWidth, destHeight) && dx + dw <= destWidth && dy + dh <= destHeight);
	}

	bool Image::correctRect(int& dx, int& dy, int& dw, int& dh, int destWidth, int destHeight)
	{
		if (dx >= destWidth || dy >= destHeight)
		{
			return false;
		}
		if (dx < 0)
		{
			dw += dx;
			dx = 0;
		}
		dw = hmin(dw, destWidth - dx);
		if (dw < 0)
		{
			return false;
		}
		if (dy < 0)
		{
			dh += dy;
			dy = 0;
		}
		dh = hmin(dh, destHeight - dy);
		if (dh < 0)
		{
			return false;
		}
		return true;
	}

	bool Image::correctRect(int& sx, int& sy, int& sw, int& sh, int srcWidth, int srcHeight, int& dx, int& dy, int destWidth, int destHeight)
	{
		if (!Image::checkRect(sx, sy, sw, sh, srcWidth, srcHeight))
		{
			return false;
		}
		if (dx < 0)
		{
			sx -= dx;
			sw += dx;
			dx = 0;
		}
		if (sx >= srcWidth || sw <= 0)
		{
			return false;
		}
		sw = hmin(sw, destWidth - dx);
		if (sw <= 0)
		{
			return false;
		}
		if (dy < 0)
		{
			sy -= dy;
			sh += dy;
			dy = 0;
		}
		if (sy >= srcHeight || sh <= 0)
		{
			return false;
		}
		sh = hmin(sh, destHeight - dy);
		if (sh <= 0)
		{
			return false;
		}
		return true;
	}

	bool Image::correctRect(int& sx, int& sy, int& sw, int& sh, int srcWidth, int srcHeight, int& dx, int& dy, int& dw, int& dh, int destWidth, int destHeight)
	{
		if (!Image::checkRect(sx, sy, sw, sh, srcWidth, srcHeight))
		{
			return false;
		}
		if (dw <= 0 || dh <= 0)
		{
			return false;
		}
		float fw = (float)sw / dw;
		if (dx < 0)
		{
			sx = (int)(sx - dx * fw);
			sw = (int)(sw + dx * fw);
			dw += dx;
			dx = 0;
		}
		if (sx >= srcWidth || dw <= 0)
		{
			return false;
		}
		int ox = dw - destWidth + dx;
		if (ox > 0)
		{
			sw = (int)(sw - ox * fw);
			dw -= ox;
		}
		if (sw <= 0 || dw <= 0)
		{
			return false;
		}
		float fh = (float)sh / dh;
		if (dy < 0)
		{
			sy = (int)(sy - dy * fh);
			sh = (int)(sh + dy * fh);
			dh += dy;
			dy = 0;
		}
		if (sy >= srcHeight || dh <= 0)
		{
			return false;
		}
		int oy = dh - destHeight + dy;
		if (oy > 0)
		{
			sh = (int)(sh - oy * fh);
			dh -= oy;
		}
		if (sh <= 0 || dh <= 0)
		{
			return false;
		}
		return true;
	}

	void Image::_getFormatIndices(Image::Format format, int* red, int* green, int* blue, int* alpha)
	{
		switch (format)
		{
		case FORMAT_RGBA:
		case FORMAT_RGBX:
			if (alpha != NULL)
			{
				*alpha = 3;
			}
			// fall-through is ok here
		case FORMAT_RGB:
			if (red != NULL)
			{
				*red = 0;
			}
			if (green != NULL)
			{
				*green = 1;
			}
			if (blue != NULL)
			{
				*blue = 2;
			}
			break;
		case FORMAT_BGRA:
		case FORMAT_BGRX:
			if (alpha != NULL)
			{
				*alpha = 3;
			}
			// fall-through is ok here
		case FORMAT_BGR:
			if (red != NULL)
			{
				*red = 2;
			}
			if (green != NULL)
			{
				*green = 1;
			}
			if (blue != NULL)
			{
				*blue = 0;
			}
			break;
		case FORMAT_ARGB:
		case FORMAT_XRGB:
			if (alpha != NULL)
			{
				*alpha = 0;
			}
			if (red != NULL)
			{
				*red = 1;
			}
			if (green != NULL)
			{
				*green = 2;
			}
			if (blue != NULL)
			{
				*blue = 3;
			}
			break;
		case FORMAT_ABGR:
		case FORMAT_XBGR:
			if (alpha != NULL)
			{
				*alpha = 0;
			}
			if (red != NULL)
			{
				*red = 3;
			}
			if (green != NULL)
			{
				*green = 2;
			}
			if (blue != NULL)
			{
				*blue = 1;
			}
			break;
		case FORMAT_ALPHA:
		case FORMAT_GRAYSCALE:
			if (alpha != NULL)
			{
				*alpha = 0;
			}
			if (red != NULL)
			{
				*red = 0;
			}
			if (green != NULL)
			{
				*green = 0;
			}
			if (blue != NULL)
			{
				*blue = 0;
			}
			break;
		}
	}

}
