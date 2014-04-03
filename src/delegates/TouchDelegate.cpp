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
#include "TouchDelegate.h"

namespace april
{
	TouchDelegate::TouchDelegate()
	{
	}

	TouchDelegate::~TouchDelegate()
	{
	}

	void TouchDelegate::onTouch(const harray<gvec2>& touches)
	{
		hlog::debug(april::logTag, "Event onTouch() was not implemented.");
	}

}
