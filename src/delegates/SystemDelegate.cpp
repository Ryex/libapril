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
#include "SystemDelegate.h"

namespace april
{
	SystemDelegate::SystemDelegate()
	{
	}

	SystemDelegate::~SystemDelegate()
	{
	}

	bool SystemDelegate::onQuit(bool canCancel)
	{
		hlog::debug(april::logTag, "Event onQuit() was not implemented.");
		return true;
	}

	void SystemDelegate::onWindowSizeChanged(int width, int height, bool fullscreen)
	{
		hlog::debug(april::logTag, "Event onWindowSizeChanged() was not implemented.");
	}

	void SystemDelegate::onWindowFocusChanged(bool focused)
	{
		hlog::debug(april::logTag, "Event onWindowFocusChanged() was not implemented.");
	}

	void SystemDelegate::onInputModeChanged(Window::InputMode inputMode)
	{
		hlog::debug(april::logTag, "Event onInputModeChanged() was not implemented.");
	}

	void SystemDelegate::onVirtualKeyboardChanged(bool visible, float heightRatio)
	{
		hlog::debug(april::logTag, "Event onVirtualKeyboardChanged() was not implemented.");
	}

	void SystemDelegate::onLowMemoryWarning()
	{
		hlog::debug(april::logTag, "Event onLowMemoryWarning() was not implemented.");
	}

}
