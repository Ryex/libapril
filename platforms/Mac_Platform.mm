/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <sys/sysctl.h>
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
#import <AppKit/NSApplication.h>
#import <AppKit/NSCursor.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSPanel.h>
#import <AppKit/NSScreen.h>
#import <AppKit/NSWindow.h>
#import <Foundation/NSString.h>

#include <gtypes/Vector2.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "Window.h"

namespace april
{
	float getMacOSVersion()
	{
#ifdef _DEBUG
		//return 10.6f; // uncomment this to test behaviour on older macs
#endif
		static float version = 0;
		if (version == 0)
		{
			SInt32 major, minor, bugfix;
			if (Gestalt(gestaltSystemVersionMajor, &major)   == noErr &&
				Gestalt(gestaltSystemVersionMinor, &minor)   == noErr &&
				Gestalt(gestaltSystemVersionBugFix, &bugfix) == noErr)
			{
				version = major + minor / 10.0f + bugfix / 100.0f;
			}
			else version = 10.3f; // just in case. < 10.4 is not supported.
		}
		return version;
	}
	
	bool platform_CursorIsVisible()
	{
		return CGCursorIsVisible();
	}
	
	void platform_cursorVisibilityUpdate()
	{
		// mac only: extra visibility handling
		NSWindow* window = [[NSApplication sharedApplication] keyWindow];
		bool shouldShow;
		
		if (!april::window->isCursorVisible())
		{
			//NSPoint 	mouseLoc = [window convertScreenToBase:[NSEvent mouseLocation]];
			//[window frame]
			NSPoint mouseLoc;
			id hideInsideView; // either NSView or NSWindow; both implement "frame" method
			if ([window contentView])
			{
				hideInsideView = [window contentView];
				mouseLoc = [window convertScreenToBase:[NSEvent mouseLocation]];
			}
			else
			{
				hideInsideView = window;
				mouseLoc = [NSEvent mouseLocation];
			}
			
			if (hideInsideView)
			{
				shouldShow = !NSPointInRect(mouseLoc, [hideInsideView frame]);
			}
			else // no view? let's presume we are in fullscreen where we should blindly honor the requests from the game
			{
				shouldShow = false;
			}
		}
		else
		{			
			shouldShow = true;
		}
		
		if (!shouldShow && CGCursorIsVisible())
		{
			CGDisplayHideCursor(kCGDirectMainDisplay);
		}
		else if (shouldShow && !CGCursorIsVisible())
		{
			CGDisplayShowCursor(kCGDirectMainDisplay);
		}
	}
	
	extern SystemInfo info;
	
	SystemInfo getSystemInfo()
	{
		if (info.locale == "")
		{
			// CPU cores
			info.cpuCores = sysconf(_SC_NPROCESSORS_ONLN);
			// RAM
			info.name = "mac";
			info.osVersion = getMacOSVersion();

			int mib [] = { CTL_HW, HW_MEMSIZE };
			int64_t value = 0;
			size_t length = sizeof(value);

			if (sysctl(mib, 2, &value, &length, NULL, 0) == -1)
				info.ram = 2048;
			else
				info.ram = value / (1024 * 1024);

			// display resolution
			NSScreen* mainScreen = [NSScreen mainScreen];
			NSRect rect = [mainScreen frame];
			info.displayResolution.set((float)rect.size.width, (float)rect.size.height);
			// display DPI
			CGSize screenSize = CGDisplayScreenSize(CGMainDisplayID());
			info.displayDpi = 25.4f * rect.size.height / screenSize.height;

			// locale
			// This code gets the prefered locale based on user's list of prefered languages against the supported languages
			// in the app bundle (the .lproj folders in the bundle)
			CFArrayRef locs = CFBundleCopyBundleLocalizations(CFBundleGetMainBundle());
			CFArrayRef preferred = CFBundleCopyPreferredLocalizationsFromArray(locs);
			CFStringRef loc = (CFStringRef) CFArrayGetValueAtIndex(preferred, 0);
			CFStringRef lang = CFLocaleCreateCanonicalLocaleIdentifierFromString(NULL, loc);
			char cstr[64 + 1];
			CFStringGetCString(lang, cstr, 64, kCFStringEncodingASCII);
			info.locale = cstr;
		}
		return info;
	}
	
	hstr getPackageName()
	{
		static hstr bundleID;
		if (bundleID == "")
		{
			NSString *bundleIdentifier = [[NSBundle mainBundle] bundleIdentifier];
			bundleID = [bundleIdentifier UTF8String];
		}
		return bundleID;
	}

	hstr getUserDataPath()
	{
		hlog::warn(april::logTag, "Cannot use getUserDataPath() on this platform.");
		return ".";
	}
	
	void messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style, hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
		// fugly implementation of showing messagebox on mac os
		// ideas:
		// * display as a sheet attached on top of the current window
		// * prioritize buttons and automatically assign slots
		// * use constants for button captions
		// * use an array with constants for button captions etc
		
		NSString *buttons[] = {@"OK", nil, nil}; // set all buttons to nil, at first, except default one, just in case
		MessageBoxButton buttonTypes[] = {MESSAGE_BUTTON_OK, (MessageBoxButton)NULL, (MessageBoxButton)NULL};
        
		if ((buttonMask & MESSAGE_BUTTON_OK) && (buttonMask & MESSAGE_BUTTON_CANCEL))
		{
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(MESSAGE_BUTTON_OK, "OK").c_str()];
			buttons[1] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(MESSAGE_BUTTON_CANCEL, "Cancel").c_str()];
            buttonTypes[0] = MESSAGE_BUTTON_OK;
            buttonTypes[1] = MESSAGE_BUTTON_CANCEL;
		}
		else if ((buttonMask & MESSAGE_BUTTON_YES) && (buttonMask & MESSAGE_BUTTON_NO) && (buttonMask & MESSAGE_BUTTON_CANCEL))
		{
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(MESSAGE_BUTTON_YES, "Yes").c_str()];
			buttons[1] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(MESSAGE_BUTTON_NO, "No").c_str()];
			buttons[2] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(MESSAGE_BUTTON_CANCEL, "Cancel").c_str()];
            buttonTypes[0] = MESSAGE_BUTTON_YES;
            buttonTypes[1] = MESSAGE_BUTTON_NO;
            buttonTypes[2] = MESSAGE_BUTTON_CANCEL;
		}
		else if ((buttonMask & MESSAGE_BUTTON_YES) && (buttonMask & MESSAGE_BUTTON_NO))
		{
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(MESSAGE_BUTTON_YES, "Yes").c_str()];
			buttons[1] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(MESSAGE_BUTTON_NO, "No").c_str()];
            buttonTypes[0] = MESSAGE_BUTTON_YES;
            buttonTypes[1] = MESSAGE_BUTTON_NO;
		}
		else if (buttonMask & MESSAGE_BUTTON_CANCEL)
		{
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(MESSAGE_BUTTON_CANCEL, "Cancel").c_str()];
            buttonTypes[0] = MESSAGE_BUTTON_CANCEL;
		}
		else if (buttonMask & MESSAGE_BUTTON_OK)
		{
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(MESSAGE_BUTTON_OK, "OK").c_str()];
            buttonTypes[0] = MESSAGE_BUTTON_OK;
		}
		else if (buttonMask & MESSAGE_BUTTON_YES)
		{
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(MESSAGE_BUTTON_YES, "Yes").c_str()];
            buttonTypes[0] = MESSAGE_BUTTON_YES;
		}
		else if (buttonMask & MESSAGE_BUTTON_NO)
		{
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(MESSAGE_BUTTON_NO, "No").c_str()];
            buttonTypes[0] = MESSAGE_BUTTON_NO;
		}
		
		NSString *titlens = [NSString stringWithUTF8String:title.c_str()];
		
		int clicked = NSRunAlertPanel(titlens, @"%@", buttons[0], buttons[1], buttons[2], [NSString stringWithUTF8String:text.c_str()]);
		switch (clicked)
		{
		case NSAlertDefaultReturn:
			clicked = 0;
			break;
		case NSAlertAlternateReturn:
			clicked = 1;
			break;
		case NSAlertOtherReturn:
			clicked = 2;
			break;
		}
        
        if (callback != NULL)
        {
            (*callback)(buttonTypes[clicked]);
		}
	}
	
}
#endif
