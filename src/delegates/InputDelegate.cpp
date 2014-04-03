/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include "InputDelegate.h"

namespace april
{
	InputDelegate::InputDelegate() : KeyboardDelegate(), MouseDelegate(), TouchDelegate(), ControllerDelegate()
	{
	}

	InputDelegate::~InputDelegate()
	{
	}

}
