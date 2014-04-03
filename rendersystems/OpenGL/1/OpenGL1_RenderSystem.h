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
/// Defines an OpenGL1 render system.

#ifdef _OPENGL1
#ifndef APRIL_OPENGL1_RENDER_SYSTEM_H
#define APRIL_OPENGL1_RENDER_SYSTEM_H

#include "OpenGL_RenderSystem.h"
#include "OpenGL_State.h"

namespace april
{
	class OpenGL1_Texture;
	class Window;

	class OpenGL1_RenderSystem : public OpenGL_RenderSystem
	{
	public:
		friend class OpenGL1_Texture;

		OpenGL1_RenderSystem();
		~OpenGL1_RenderSystem();

		void assignWindow(Window* window);
		
		int getMaxTextureSize();

	protected:
		void _setupDefaultParameters();

		Texture* _createTexture(bool fromResource);

		void _setVertexPointer(int stride, const void* pointer);

		void _setTextureBlendMode(BlendMode mode);

#ifdef _WIN32
		HGLRC hRC;

		void _releaseWindow();
#endif

	};
	
}

#endif
#endif
