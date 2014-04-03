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
#include "Keys.h"
#include "MouseDelegate.h"

namespace april
{
	MouseDelegate::MouseDelegate()
	{
	}

	MouseDelegate::~MouseDelegate()
	{
	}

	void MouseDelegate::onMouseDown(april::Key keyCode)
	{
		hlog::debug(april::logTag, "Event onMouseDown() was not implemented.");
	}

	void MouseDelegate::onMouseUp(april::Key keyCode)
	{
		hlog::debug(april::logTag, "Event onMouseUp() was not implemented.");
	}

	void MouseDelegate::onMouseCancel(april::Key keyCode)
	{
		hlog::debug(april::logTag, "Event onMouseCancel() was not implemented.");
	}

	void MouseDelegate::onMouseMove()
	{
		hlog::debug(april::logTag, "Event onMouseMove() was not implemented.");
	}

	void MouseDelegate::onMouseScroll(float x, float y)
	{
		hlog::debug(april::logTag, "Event onMouseScroll() was not implemented.");
	}

}
