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
/// Defines a WinRT window.

#ifdef _WINRT_WINDOW
#ifndef APRIL_WINRT_WINDOW_H
#define APRIL_WINRT_WINDOW_H

#include <gtypes/Matrix4.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Color.h"
#include "Timer.h"
#include "Window.h"
#include "WinRT.h"

#define WINRT_SNAPPED_VIEW_WIDTH 320 // as defined by Microsoft
#define WINRT_SNAPPED_VIEW_UNUSED (WINRT_SNAPPED_VIEW_WIDTH + 22)

#define WINRT_ALLOW_FILLED_VIEW "allow_filled_view"
#define WINRT_USE_CUSTOM_SNAPPED_VIEW "use_custom_snapped_view"
#define WINRT_DELAY_SPLASH "delay_splash"
#define WINRT_CURSOR_MAPPINGS "cursor_mappings"
#define WINRT_VIEW_STATE "view_state"
#define WINRT_VIEW_STATE_FILLED "filled"
#define WINRT_VIEW_STATE_SNAPPED "snapped"
#define WINRT_VIEW_STATE_FULLSCREEN "fullscreen"
#define WINP8_BACK_BUTTON_SYSTEM_HANDLING "back_button_system_handling"

namespace april
{
	class Texture;
	
	class WinRT_Window : public Window
	{
	public:
		WinRT_Window();
		~WinRT_Window();
		
		bool create(int w, int h, bool fullscreen, chstr title, Window::Options options);
		void unassign();
		
		hstr getParam(chstr param);
		void setParam(chstr param, chstr value);
		
		void setTitle(chstr title);
		void setCursorVisible(bool value);
		void setCursorFilename(chstr value);
		HL_DEFINE_GET(int, width, Width);
		HL_DEFINE_GET(int, height, Height);
		void* getBackendId();

		void setResolution(int w, int h, bool fullscreen);
		void presentFrame();
		void checkEvents();
		
		void beginKeyboardHandling();
		void terminateKeyboardHandling();
		void changeSize(int w, int h); // required override instead of normal size changing
		
	protected:
		int width;
		int height;
		float delaySplash;
		bool allowFilledView;
		bool useCustomSnappedView;
		bool backButtonSystemHandling;
		hmap<hstr, unsigned int> cursorMappings;

		hstr _findCursorFile();
		
	};
	
}

#endif
#endif
