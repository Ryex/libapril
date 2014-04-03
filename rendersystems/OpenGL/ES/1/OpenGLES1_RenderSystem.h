/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
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
/// Defines an OpenGLES1 render system.

#ifdef _OPENGLES1
#ifndef APRIL_OPENGLES1_RENDER_SYSTEM_H
#define APRIL_OPENGLES1_RENDER_SYSTEM_H

#include <hltypes/hstring.h>

#include "Color.h"
#include "OpenGLES_RenderSystem.h"
#include "Texture.h"

namespace april
{
	class OpenGLES1_Texture;

	class OpenGLES1_RenderSystem : public OpenGLES_RenderSystem
	{
	public:
		friend class OpenGLES1_Texture;

		OpenGLES1_RenderSystem();
		~OpenGLES1_RenderSystem();

	protected:
		Texture* _createTexture(bool fromResource);

		void _setVertexPointer(int stride, const void* pointer);

	};
	
}

#endif
#endif
