/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _OPENGLES1
#include "OpenGLES1_Texture.h"

namespace april
{
	OpenGLES1_Texture::OpenGLES1_Texture(bool fromResource) : OpenGLES_Texture(fromResource)
	{
	}

	OpenGLES1_Texture::~OpenGLES1_Texture()
	{
	}

}
#endif
