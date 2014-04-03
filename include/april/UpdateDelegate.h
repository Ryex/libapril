/// @file
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
/// Defines a delegate for the update callback for the render loop.

#ifndef APRIL_UPDATE_DELEGATE_H
#define APRIL_UPDATE_DELEGATE_H

#include "aprilExport.h"

namespace april
{
	class aprilExport UpdateDelegate
	{
	public:
		UpdateDelegate();
		virtual ~UpdateDelegate();

		virtual bool onUpdate(float timeSinceLastFrame);

	};

}
#endif
