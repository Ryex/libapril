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
/// Defines an Android JNI window.

#ifdef _ANDROIDJNI_WINDOW
#ifndef APRIL_ANDROID_JNI_WINDOW_H
#define APRIL_ANDROID_JNI_WINDOW_H

#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Timer.h"
#include "Window.h"

namespace april
{
	class AndroidJNI_Window : public Window
	{
	public:
		AndroidJNI_Window();
		~AndroidJNI_Window();
		bool create(int w, int h, bool fullscreen, chstr title, Window::Options options);
		
		void setTitle(chstr title) { }
		bool isCursorVisible() { return false; }
		void setCursorVisible(bool value) { }
		HL_DEFINE_GET(int, width, Width);
		HL_DEFINE_GET(int, height, Height);
		void* getBackendId();
		
		void enterMainLoop();
		void presentFrame();
		
		void queueTouchEvent(MouseEventType type, gvec2 position, int index);
		void queueControllerEvent(ControllerEventType type, Button buttonCode);

		void beginKeyboardHandling();
		void terminateKeyboardHandling();

		void handleFocusChangeEvent(bool focused);
		void handleActivityChangeEvent(bool active);
		
	protected:
		int width;
		int height;
		bool forcedFocus;
		
	};

}
#endif
#endif