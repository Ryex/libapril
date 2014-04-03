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
/// Defines platform specific functionality.

#ifndef APRIL_PLATFORM_H
#define APRIL_PLATFORM_H

#include <gtypes/Vector2.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"

namespace april
{
	struct aprilExport SystemInfo
	{
		hstr name;
		hstr architecture;
		float osVersion;
		int cpuCores; // number of CPU cores or separate CPU units
		int ram; // how many MB of RAM does the host system have in total
		gvec2 displayResolution;
		int displayDpi;
		hstr locale; // current system locale code
		
		SystemInfo();
		~SystemInfo();
		
	};
	
	enum MessageBoxButton
	{
		MESSAGE_BUTTON_OK = 1,
		MESSAGE_BUTTON_CANCEL = 2,
		MESSAGE_BUTTON_YES = 4,
		MESSAGE_BUTTON_NO = 8,
		MESSAGE_BUTTON_OK_CANCEL = MESSAGE_BUTTON_OK | MESSAGE_BUTTON_CANCEL,
		MESSAGE_BUTTON_YES_NO = MESSAGE_BUTTON_YES | MESSAGE_BUTTON_NO,
		MESSAGE_BUTTON_YES_NO_CANCEL = MESSAGE_BUTTON_YES_NO | MESSAGE_BUTTON_CANCEL

	};
	
	enum MessageBoxStyle
	{
		MESSAGE_STYLE_NORMAL = 0,
		MESSAGE_STYLE_INFO = 1,
		MESSAGE_STYLE_WARNING = 2,
		MESSAGE_STYLE_CRITICAL = 3,
		MESSAGE_STYLE_QUESTION = 4,
		MESSAGE_STYLE_MODAL = 8,
		MESSAGE_STYLE_TERMINATE_ON_DISPLAY = 16

	};

	DEPRECATED_ATTRIBUTE static const MessageBoxButton AMSGBTN_NULL = (MessageBoxButton)NULL;
	DEPRECATED_ATTRIBUTE static const MessageBoxButton AMSGBTN_OK = MESSAGE_BUTTON_OK;
	DEPRECATED_ATTRIBUTE static const MessageBoxButton AMSGBTN_CANCEL = MESSAGE_BUTTON_CANCEL;
	DEPRECATED_ATTRIBUTE static const MessageBoxButton AMSGBTN_YES = MESSAGE_BUTTON_YES;
	DEPRECATED_ATTRIBUTE static const MessageBoxButton AMSGBTN_NO = MESSAGE_BUTTON_NO;
	DEPRECATED_ATTRIBUTE static const MessageBoxButton AMSGBTN_OKCANCEL = MESSAGE_BUTTON_OK_CANCEL;
	DEPRECATED_ATTRIBUTE static const MessageBoxButton AMSGBTN_YESNO = MESSAGE_BUTTON_YES_NO;
	DEPRECATED_ATTRIBUTE static const MessageBoxButton AMSGBTN_YESNOCANCEL = MESSAGE_BUTTON_YES_NO_CANCEL;

	DEPRECATED_ATTRIBUTE static const MessageBoxStyle AMSGSTYLE_PLAIN = MESSAGE_STYLE_NORMAL;
	DEPRECATED_ATTRIBUTE static const MessageBoxStyle AMSGSTYLE_INFORMATION = MESSAGE_STYLE_INFO;
	DEPRECATED_ATTRIBUTE static const MessageBoxStyle AMSGSTYLE_WARNING = MESSAGE_STYLE_WARNING;
	DEPRECATED_ATTRIBUTE static const MessageBoxStyle AMSGSTYLE_CRITICAL = MESSAGE_STYLE_CRITICAL;
	DEPRECATED_ATTRIBUTE static const MessageBoxStyle AMSGSTYLE_QUESTION = MESSAGE_STYLE_QUESTION;
	DEPRECATED_ATTRIBUTE static const MessageBoxStyle AMSGSTYLE_MODAL = MESSAGE_STYLE_MODAL;
	DEPRECATED_ATTRIBUTE static const MessageBoxStyle AMSGSTYLE_TERMINATEAPPONDISPLAY = MESSAGE_STYLE_TERMINATE_ON_DISPLAY;
		
	aprilFnExport SystemInfo getSystemInfo();
	aprilFnExport hstr getPackageName();
	aprilFnExport hstr getUserDataPath();
	aprilFnExport void messageBox(chstr title, chstr text, MessageBoxButton buttonMask = MESSAGE_BUTTON_OK, MessageBoxStyle style = MESSAGE_STYLE_NORMAL,
		hmap<MessageBoxButton, hstr> customButtonTitles = hmap<MessageBoxButton, hstr>(), void(*callback)(MessageBoxButton) = NULL);

	void _makeButtonLabels(hstr* ok, hstr* yes, hstr* no, hstr* cancel,
		MessageBoxButton buttonMask, hmap<MessageBoxButton, hstr> customButtonTitles);

	void messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask = MESSAGE_BUTTON_OK, MessageBoxStyle style = MESSAGE_STYLE_NORMAL,
		hmap<MessageBoxButton, hstr> customButtonTitles = hmap<MessageBoxButton, hstr>(), void(*callback)(MessageBoxButton) = NULL);

}

#endif
