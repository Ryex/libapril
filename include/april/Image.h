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
/// Defines a generic image source.

#ifndef APRIL_IMAGE_H
#define APRIL_IMAGE_H

#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/hsbase.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Color.h"

namespace april
{
	class Color;

	class aprilExport Image
	{
	public:
		/// @note Some formats are intended to improve speed with the underlying engine if really needed. *X* formats are always 4 BPP even if that byte is not used.
		enum Format
		{
			FORMAT_INVALID,
			FORMAT_RGBA,
			FORMAT_ARGB,
			FORMAT_BGRA,
			FORMAT_ABGR,
			FORMAT_RGBX,
			FORMAT_XRGB,
			FORMAT_BGRX,
			FORMAT_XBGR,
			FORMAT_RGB,
			FORMAT_BGR,
			FORMAT_ALPHA,
			FORMAT_GRAYSCALE,
			FORMAT_PALETTE
		};

		unsigned char* data;
		int w;
		int h;
		Format format;
		int internalFormat; // needed for special platform dependent formats, usually used internally only
		int compressedSize;

		~Image();
		
		int getBpp();
		int getByteSize();
		bool isValid();

		bool clear();
		Color getPixel(int x, int y);
		bool setPixel(int x, int y, Color color);
		Color getInterpolatedPixel(float x, float y);
		bool fillRect(int x, int y, int w, int h, Color color);
		bool copyPixelData(unsigned char** output, Format format);
		bool write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		bool writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		bool blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		bool blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		bool rotateHue(int x, int y, int w, int h, float degrees);
		bool saturate(int x, int y, int w, int h, float factor);
		bool invert(int x, int y, int w, int h);
		/// @note srcData must be the same width and height as the image
		bool insertAlphaMap(unsigned char* srcData, Format srcFormat, unsigned char median, int ambiguity);
		
		Color getPixel(gvec2 position);
		bool setPixel(gvec2 position, Color color);
		Color getInterpolatedPixel(gvec2 position);
		bool fillRect(grect rect, Color color);
		bool copyPixelData(unsigned char** output);
		bool write(int sx, int sy, int sw, int sh, int dx, int dy, Image* other);
		bool write(grect srcRect, gvec2 destPosition, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		bool write(grect srcRect, gvec2 destPosition, Image* other);
		bool writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Image* other);
		bool writeStretch(grect srcRect, grect destRect, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		bool writeStretch(grect srcRect, grect destRect, Image* other);
		bool blit(int sx, int sy, int sw, int sh, int dx, int dy, Image* other, unsigned char alpha = 255);
		bool blit(grect srcRect, gvec2 destPosition, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		bool blit(grect srcRect, gvec2 destPosition, Image* other, unsigned char alpha = 255);
		bool blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Image* other, unsigned char alpha = 255);
		bool blitStretch(grect srcRect, grect destRect, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		bool blitStretch(grect srcRect, grect destRect, Image* other, unsigned char alpha = 255);
		bool rotateHue(grect rect, float degrees);
		bool saturate(grect rect, float factor);
		bool invert(grect rect);
		bool insertAlphaMap(Image* image, Format srcFormat, unsigned char median, int ambiguity);

		static Image* createFromResource(chstr filename);
		static Image* createFromResource(chstr filename, Format format);
		static Image* createFromFile(chstr filename);
		static Image* createFromFile(chstr filename, Format format);
		static Image* create(int w, int h, unsigned char* data, Format format);
		static Image* create(int w, int h, Color color, Format format);
		static Image* create(Image* other);

		static int getFormatBpp(Format format);

		static Color getPixel(int x, int y, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat);
		static bool setPixel(int x, int y, Color color, unsigned char* destData, int destWidth, int destHeight, Format destFormat);
		static Color getInterpolatedPixel(float x, float y, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat);
		static bool fillRect(int x, int y, int w, int h, Color color, unsigned char* destData, int destWidth, int destHeight, Format destFormat);
		static bool write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Format destFormat);
		static bool writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Format destFormat);
		static bool blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Format destFormat, unsigned char alpha = 255);
		static bool blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Format destFormat, unsigned char alpha = 255);
		static bool rotateHue(int x, int y, int w, int h, float degrees, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		static bool saturate(int x, int y, int w, int h, float factor, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		static bool invert(int x, int y, int w, int h, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		static bool insertAlphaMap(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char* destData, Format destFormat, unsigned char median, int ambiguity);

		/// @param[in] preventCopy If true, will make a copy even if source and destination formats are the same.
		static bool convertToFormat(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char** destData, Format destFormat, bool preventCopy = true);
		/// @brief Checks if an image format conversion is needed.
		/// @param[in] preventCopy If true, will return false if source and destination formats are the same.
		/// @note Helps to determine whether there is a need to convert an image format into another. It can be helpful to avoid conversion from e.g. RGBA to RGBX if the GPU ignores the X anyway.
		static bool needsConversion(Format srcFormat, Format destFormat, bool preventCopy = true);
		
		static bool checkRect(int dx, int dy, int destWidth, int destHeight);
		static bool checkRect(int dx, int dy, int dw, int dh, int destWidth, int destHeight);
		static bool correctRect(int& dx, int& dy, int& dw, int& dh, int destWidth, int destHeight);
		static bool correctRect(int& sx, int& sy, int& sw, int& sh, int srcWidth, int srcHeight, int& dx, int& dy, int destWidth, int destHeight);
		static bool correctRect(int& sx, int& sy, int& sw, int& sh, int srcWidth, int srcHeight, int& dx, int& dy, int& dw, int& dh, int destWidth, int destHeight);

		DEPRECATED_ATTRIBUTE static Image* load(chstr filename) { return Image::createFromResource(filename); }
		DEPRECATED_ATTRIBUTE static Image* load(chstr filename, Format format) { return Image::createFromResource(filename, format); }

	protected:
		Image();

		static Image* _loadPng(hsbase& stream, int size);
		static Image* _loadPng(hsbase& stream);
		static Image* _loadJpg(hsbase& stream, int size);
		static Image* _loadJpg(hsbase& stream);
		static Image* _loadJpt(hsbase& stream);

		static void _getFormatIndices(Format format, int* red, int* green, int* blue, int* alpha);

		static bool _convertFrom1Bpp(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char** destData, Format destFormat);
		static bool _convertFrom3Bpp(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char** destData, Format destFormat);
		static bool _convertFrom4Bpp(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char** destData, Format destFormat);

		static bool _blitFrom1Bpp(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Format destFormat, unsigned char alpha);
		static bool _blitFrom3Bpp(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Format destFormat, unsigned char alpha);
		static bool _blitFrom4Bpp(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Format destFormat, unsigned char alpha);

	};
	
}

#endif
