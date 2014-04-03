/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.31
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic texture.

#ifndef APRIL_TEXTURE_H
#define APRIL_TEXTURE_H

#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Color.h"
#include "Image.h"

namespace april
{
	class Image;
	
	class aprilExport Texture
	{
	public:
		friend class RenderSystem;

		enum Type
		{
			/// @brief Resides in RAM and on GPU, can be modified. Best used for manually created textures or loaded from files which will be modified.
			TYPE_MANAGED = 1,
			/// @brief Cannot be modified or read. Texture with manual data will have a copy of the data in RAM, files will be reloaded from persistent memory.
			TYPE_IMMUTABLE = 2,
			/// @brief Used for feeding the GPU texture data constantly (e.g. video). It has no local RAM copy for when the rendering context is lost and cannot be restored.
			TYPE_VOLATILE = 3
		};

		enum Filter
		{
			FILTER_NEAREST = 1,
			FILTER_LINEAR = 2,
			FILTER_UNDEFINED = 0x7FFFFFFF
		};

		enum AddressMode
		{
			ADDRESS_WRAP = 0,
			ADDRESS_CLAMP = 1,
			ADDRESS_UNDEFINED = 0x7FFFFFFF
		};

		DEPRECATED_ATTRIBUTE static Image::Format FORMAT_ALPHA;
		DEPRECATED_ATTRIBUTE static Image::Format FORMAT_ARGB;

		Texture(bool fromResource);
		virtual ~Texture();
		virtual bool load();
		virtual void unload() = 0;

		HL_DEFINE_GET(hstr, filename, Filename);
		HL_DEFINE_GET(Image::Format, format, Format);
		HL_DEFINE_GETSET(Filter, filter, Filter);
		HL_DEFINE_GETSET(AddressMode, addressMode, AddressMode);
		HL_DEFINE_IS(fromResource, FromResource);
		int getWidth();
		int getHeight();
		int getBpp();
		int getByteSize();

		virtual bool isLoaded() = 0;
		
		bool clear();
		Color getPixel(int x, int y);
		Color getPixel(gvec2 position);
		bool setPixel(int x, int y, Color color);
		bool setPixel(gvec2 position, Color color);
		Color getInterpolatedPixel(float x, float y);
		Color getInterpolatedPixel(gvec2 position);
		bool fillRect(int x, int y, int w, int h, Color color);
		bool fillRect(grect rect, Color color);
		bool copyPixelData(unsigned char** output, Image::Format format);
		bool copyPixelData(unsigned char** output);
		bool write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		bool write(int sx, int sy, int sw, int sh, int dx, int dy, Texture* texture);
		bool write(grect srcRect, gvec2 destPosition, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		bool write(grect srcRect, gvec2 destPosition, Texture* texture);
		bool write(int sx, int sy, int sw, int sh, int dx, int dy, Image* image);
		bool write(grect srcRect, gvec2 destPosition, Image* image);
		bool writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		bool writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Texture* texture);
		bool writeStretch(grect srcRect, grect destRect, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		bool writeStretch(grect srcRect, grect destRect, Texture* texture);
		bool writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Image* image);
		bool writeStretch(grect srcRect, grect destRect, Image* image);
		bool blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		bool blit(int sx, int sy, int sw, int sh, int dx, int dy, Texture* texture, unsigned char alpha = 255);
		bool blit(grect srcRect, gvec2 destPosition, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		bool blit(grect srcRect, gvec2 destPosition, Texture* texture, unsigned char alpha = 255);
		bool blit(int sx, int sy, int sw, int sh, int dx, int dy, Image* image, unsigned char alpha = 255);
		bool blit(grect srcRect, gvec2 destPosition, Image* image, unsigned char alpha = 255);
		bool blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		bool blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Texture* texture, unsigned char alpha = 255);
		bool blitStretch(grect srcRect, grect destRect, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		bool blitStretch(grect srcRect, grect destRect, Texture* texture, unsigned char alpha = 255);
		bool blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Image* image, unsigned char alpha = 255);
		bool blitStretch(grect srcRect, grect destRect, Image* image, unsigned char alpha = 255);
		bool rotateHue(int x, int y, int w, int h, float degrees);
		bool rotateHue(grect rect, float degrees);
		bool saturate(int x, int y, int w, int h, float factor);
		bool saturate(grect rect, float factor);
		bool invert(int x, int y, int w, int h);
		bool invert(grect rect);
		/// @note srcData must be the same width and height as the image
		bool insertAlphaMap(unsigned char* srcData, Image::Format srcFormat, unsigned char median, int ambiguity);
		bool insertAlphaMap(Texture* texture, unsigned char median, int ambiguity);
		bool insertAlphaMap(Image* image, unsigned char median, int ambiguity);

	protected:
		struct Lock
		{
		public:
			void* systemBuffer;
			int x;
			int y;
			int w;
			int h;
			int dx;
			int dy;
			unsigned char* data;
			int dataWidth;
			int dataHeight;
			Image::Format format;
			bool locked;
			bool failed;
			bool renderTarget;

			Lock();
			~Lock();

			void activateFail();
			void activateLock(int x, int y, int w, int h, int dx, int dy, unsigned char* data, int dataWidth, int dataHeight, Image::Format format);
			void activateRenderTarget(int x, int y, int w, int h, int dx, int dy, unsigned char* data, int dataWidth, int dataHeight, Image::Format format);

		};

		hstr filename;
		Type type;
		Image::Format format;
		unsigned int dataFormat; // used internally for special image data formatting
		int width;
		int height;
		Filter filter;
		AddressMode addressMode;
		unsigned char* data;
		bool fromResource;

		virtual bool _create(chstr filename, Type type);
		virtual bool _create(chstr filename, Image::Format format, Type type);
		virtual bool _create(int w, int h, unsigned char* data, Image::Format format, Type type);
		virtual bool _create(int w, int h, Color color, Image::Format format, Type type);

		virtual bool _createInternalTexture(unsigned char* data, int size, Type type) = 0;
		virtual void _assignFormat() = 0;

		hstr _getInternalName();

		Lock _tryLock(int x, int y, int w, int h);
		Lock _tryLock();
		bool _unlock(Lock lock, bool update);
		virtual Lock _tryLockSystem(int x, int y, int w, int h) = 0;
		virtual bool _unlockSystem(Lock& lock, bool update) = 0;
		bool _uploadDataToGpu(int x, int y, int w, int h);
		virtual bool _uploadToGpu(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat) = 0;

	};
	
}

#endif
