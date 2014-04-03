/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "KeyboardDelegate.h"

namespace april
{
	KeyboardDelegate::KeyboardDelegate()
	{
	}

	KeyboardDelegate::~KeyboardDelegate()
	{
	}

	void KeyboardDelegate::onKeyDown(Key keyCode)
	{
		hlog::debug(april::logTag, "Event onKeyDown() was not implemented.");
	}

	void KeyboardDelegate::onKeyUp(Key keyCode)
	{
		hlog::debug(april::logTag, "Event onKeyUp() was not implemented.");
	}

	void KeyboardDelegate::onChar(unsigned int charCode)
	{
		hlog::debug(april::logTag, "Event onChar() was not implemented.");
	}

}
