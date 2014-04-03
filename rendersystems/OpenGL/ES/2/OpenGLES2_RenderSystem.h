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
/// Defines an OpenGLES2 render system.

#ifdef _OPENGLES2
#ifndef APRIL_OPENGLES2_RENDER_SYSTEM_H
#define APRIL_OPENGLES2_RENDER_SYSTEM_H

#include <hltypes/hstring.h>

#include "Color.h"
#include "OpenGLES_RenderSystem.h"
#include "Texture.h"

namespace april
{
	class OpenGLES1_Texture;

	class OpenGLES2_RenderSystem : public OpenGLES_RenderSystem
	{
	public:
		friend class OpenGLES1_Texture;

		OpenGLES2_RenderSystem();
		~OpenGLES2_RenderSystem();

	protected:
		Texture* _createTexture(bool fromResource);

		void _setVertexPointer(int stride, const void* pointer);

	};
	
}
#endif
#endif
