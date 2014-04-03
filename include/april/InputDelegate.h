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
/// Defines a delegate for all input callbacks.

#ifndef APRIL_INPUT_DELEGATE_H
#define APRIL_INPUT_DELEGATE_H

#include "aprilExport.h"
#include "ControllerDelegate.h"
#include "KeyboardDelegate.h"
#include "MouseDelegate.h"
#include "TouchDelegate.h"

namespace april
{
	class aprilExport InputDelegate : public KeyboardDelegate, public MouseDelegate, public TouchDelegate, public ControllerDelegate
	{
	public:
		InputDelegate();
		~InputDelegate();

	};

}
#endif
