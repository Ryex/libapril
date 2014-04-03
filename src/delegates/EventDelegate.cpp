/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include "EventDelegate.h"

namespace april
{
	EventDelegate::EventDelegate() : InputDelegate(), UpdateDelegate(), SystemDelegate()
	{
	}

	EventDelegate::~EventDelegate()
	{
	}

}
