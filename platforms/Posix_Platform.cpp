/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _UNIX
#include <gtypes/Vector2.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Platform.h"

namespace april
{
	extern SystemInfo info;
	
	SystemInfo getSystemInfo()
	{
		if (info.locale == "")
		{
			info.cpuCores = 1; // TODO
			info.displayResolution.set(1024.0f, 768.0f); // TODO
			info.displayDpi = 96; // TODO
			//info.ram = 1024; // TODO
			info.locale = "en"; // TODO
		}
		return info;
	}

	hstr getPackageName()
	{
		hlog::warn(april::logTag, "Cannot use getPackageName() on this platform.");
		return "";
	}

	hstr getUserDataPath()
	{
		hlog::error(april::logTag, "Not implemented.");
		return ".";
	}
	
	MessageBoxButton messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style, hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
		// TODO
		return MESSAGE_BUTTON_OK;
	}

}
#endif
