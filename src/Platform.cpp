/// @file
/// @author  Ivan Vucica
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hmap.h>
#include <hltypes/hstring.h>
#include <stdlib.h>
#include "Platform.h"
#include "Window.h"

namespace april
{
	SystemInfo info;

	SystemInfo::SystemInfo()
	{
		this->name = "";
#ifdef _ARM
		this->architecture = "ARM";
#elif defined(_X64)
		this->architecture = "x64";
#else
		this->architecture = "x86";
#endif
		this->osVersion = 1.0f;
		this->cpuCores = 1;
		this->ram = 256;
		this->displayDpi = 0;
		this->locale = "";
	}
	
	SystemInfo::~SystemInfo()
	{
	}

	void messageBox(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style,
		hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
		MessageBoxStyle passedStyle = style;
		if (style & MESSAGE_STYLE_TERMINATE_ON_DISPLAY)
		{
			if (window != NULL)
			{
#if !defined(_IOS) && !defined(_COCOA_WINDOW)
				window->terminateMainLoop();
				window->destroy();
#endif
#ifdef _COCOA_WINDOW
				window->destroy();
#endif
			}
			passedStyle = (MessageBoxStyle)(passedStyle | MESSAGE_STYLE_MODAL);
		}
		messageBox_platform(title, text, buttonMask, passedStyle, customButtonTitles, callback);
		if (style & MESSAGE_STYLE_TERMINATE_ON_DISPLAY)
		{
			exit(0);
		}
	}

	void _makeButtonLabels(hstr* ok, hstr* yes, hstr* no, hstr* cancel,
		MessageBoxButton buttonMask, hmap<MessageBoxButton, hstr> customButtonTitles)
	{
		if ((buttonMask & MESSAGE_BUTTON_OK) && (buttonMask & MESSAGE_BUTTON_CANCEL))
		{
			*ok = customButtonTitles.try_get_by_key(MESSAGE_BUTTON_OK, "OK");
			*cancel = customButtonTitles.try_get_by_key(MESSAGE_BUTTON_CANCEL, "Cancel");
		}
		else if ((buttonMask & MESSAGE_BUTTON_YES) && (buttonMask & MESSAGE_BUTTON_NO) && (buttonMask & MESSAGE_BUTTON_CANCEL))
		{
			*yes = customButtonTitles.try_get_by_key(MESSAGE_BUTTON_YES, "Yes");
			*no = customButtonTitles.try_get_by_key(MESSAGE_BUTTON_NO, "No");
			*cancel = customButtonTitles.try_get_by_key(MESSAGE_BUTTON_CANCEL, "Cancel");
		}
		else if (buttonMask & MESSAGE_BUTTON_OK)
		{
			*ok = customButtonTitles.try_get_by_key(MESSAGE_BUTTON_OK, "OK");
		}
		else if ((buttonMask & MESSAGE_BUTTON_YES) && (buttonMask & MESSAGE_BUTTON_NO))
		{
			*yes = customButtonTitles.try_get_by_key(MESSAGE_BUTTON_YES, "Yes");
			*no = customButtonTitles.try_get_by_key(MESSAGE_BUTTON_NO, "No");
		}
	}

}
