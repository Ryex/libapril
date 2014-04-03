/// @file
/// @author  Kresimir Spes
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a MacOSX window using Apple Cocoa API.

#ifndef APRIL_MAC_WINDOW_H
#define APRIL_MAC_WINDOW_H

#include "Window.h"

namespace april
{
	class Mac_Window : public Window
	{
	public:
		Mac_Window();
		~Mac_Window();

		virtual int getWidth();
		virtual int getHeight();
		virtual void* getBackendId();
		
		bool create(int w, int h, bool fullscreen, chstr title, Window::Options options);
		bool destroy();
		
		void setTitle(chstr title);
		gvec2 getCursorPosition();
		hstr getParam(chstr param);
		void setParam(chstr param, chstr value);
		
		void updateCursorPosition(gvec2& pos);
		bool isCursorVisible();
		void setCursorVisible(bool visible);
		bool isCursorInside();

        void presentFrame();
		bool updateOneFrame();
		void terminateMainLoop();

		void setResolution(int w, int h, bool fullscreen);
		void setFullscreenFlag(bool value);

		void OnAppGainedFocus();
		void OnAppLostFocus();
		
		void onFocusChanged(bool value);

		bool retainLoadingOverlay;
		bool fastHideLoadingOverlay;
		bool ignoreUpdate;
		bool splashScreenFadeout;
		bool fpsCounter;
		hstr fpsTitle;
	};
	
}
extern april::Mac_Window* aprilWindow;

bool isPreLion();
bool isLionOrNewer();

#endif
