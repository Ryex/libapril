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
/// Defines a generic DirectX texture.

#ifdef _DIRECTX
#ifndef APRIL_DIRECTX_TEXTURE_H
#define APRIL_DIRECTX_TEXTURE_H

#include "Texture.h"

namespace april
{
	class DirectX_Texture : public Texture
	{
	public:
		DirectX_Texture(bool fromResource);
		~DirectX_Texture();

	};

}
#endif
#endif
