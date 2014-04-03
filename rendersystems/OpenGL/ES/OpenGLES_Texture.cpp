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

#ifdef _OPENGLES
#include <hltypes/hstring.h>

#include "Color.h"
#include "OpenGLES_Texture.h"

namespace april
{
	OpenGLES_Texture::OpenGLES_Texture(bool fromResource) : OpenGL_Texture(fromResource)
	{
	}

	OpenGLES_Texture::~OpenGLES_Texture()
	{
	}

}
#endif
