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
#include "ControllerDelegate.h"

namespace april
{
	ControllerDelegate::ControllerDelegate()
	{
	}

	ControllerDelegate::~ControllerDelegate()
	{
	}

	void ControllerDelegate::onButtonDown(Button buttonCode)
	{
		hlog::debug(april::logTag, "Event onButtonDown() was not implemented.");
	}

	void ControllerDelegate::onButtonUp(Button buttonCode)
	{
		hlog::debug(april::logTag, "Event onButtonUp() was not implemented.");
	}

}
