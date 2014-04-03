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
/// Defines an OpenKODE window.

#ifdef _OPENKODE_WINDOW
#ifndef APRIL_OPENKODE_WINDOW_H
#define APRIL_OPENKODE_WINDOW_H
#include <KD/kd.h>

#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "egl.h"
#include "Window.h"

namespace april
{
	class aprilExport OpenKODE_Window : public Window
	{
		float launchDelay;
	public:
		OpenKODE_Window();
		~OpenKODE_Window();
		bool create(int w, int h, bool fullscreen, chstr title, Window::Options options);
		bool destroy();
		hstr getParam(chstr param);
		void setParam(chstr param, chstr value);
		int getWidth();
		int getHeight();
		void setTitle(chstr title);
		bool isCursorVisible();
		void setCursorVisible(bool value);
		void* getBackendId();
		void setResolution(int w, int h, bool fullscreen);

		void handleActivityChangeEvent(bool active);

		bool updateOneFrame();
		void presentFrame();
		void checkEvents();

		void beginKeyboardHandling();
		void terminateKeyboardHandling();

	protected:
		int _getAprilTouchIndex(int kdIndex);
		
		bool kdTouches[16];
		KDWindow* kdWindow;

		bool _isMousePointer();
		bool _processEvent(const KDEvent* evt);

	};

}
#endif
#endif