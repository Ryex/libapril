/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _DIRECTX
#include "DirectX_Texture.h"

namespace april
{
	DirectX_Texture::DirectX_Texture(bool fromResource) : Texture(fromResource)
	{
	}

	DirectX_Texture::~DirectX_Texture()
	{
	}

}
#endif
