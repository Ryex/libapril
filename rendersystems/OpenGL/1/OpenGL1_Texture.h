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
/// Defines an OpenGL1 specific texture.

#ifdef _OPENGL1
#ifndef APRIL_OPENGL1_TEXTURE_H
#define APRIL_OPENGL1_TEXTURE_H

#include "OpenGL_Texture.h"

namespace april
{
	class OpenGL1_Texture : public OpenGL_Texture
	{
	public:
		OpenGL1_Texture(bool fromResource);
		~OpenGL1_Texture();

	};

}

#endif
#endif
