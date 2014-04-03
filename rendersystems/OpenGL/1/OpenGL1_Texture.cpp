/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _OPENGL1
#include "OpenGL1_Texture.h"

namespace april
{
	OpenGL1_Texture::OpenGL1_Texture(bool fromResource) : OpenGL_Texture(fromResource)
	{
	}

	OpenGL1_Texture::~OpenGL1_Texture()
	{
	}

}

#endif
