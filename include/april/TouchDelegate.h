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
/// Defines a delegate for the touch input callbacks.

#ifndef APRIL_TOUCH_DELEGATE_H
#define APRIL_TOUCH_DELEGATE_H

#include <gtypes/Vector2.h>
#include <hltypes/harray.h>

#include "aprilExport.h"

namespace april
{
	class aprilExport TouchDelegate
	{
	public:
		TouchDelegate();
		virtual ~TouchDelegate();

		virtual void onTouch(const harray<gvec2>& touches);

	};

}
#endif
