/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 3.33
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a Win32 window.

#ifdef _WIN32_WINDOW
#ifndef APRIL_WIN32_WINDOW_H
#define APRIL_WIN32_WINDOW_H

#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Timer.h"
#include "Window.h"

namespace april
{
	class Win32_Window : public Window
	{
	public:
		Win32_Window();
		~Win32_Window();
		bool create(int w, int h, bool fullscreen, chstr title, Window::Options options);
		bool destroy();

		void setTitle(chstr title);
		bool isCursorVisible();
		void setCursorVisible(bool value);
		void setCursorFilename(chstr value);
		int getWidth();
		int getHeight();
		void* getBackendId();
		void setResolution(int w, int h, bool fullscreen);

		bool updateOneFrame();
		void presentFrame();
		void checkEvents();

		void beginKeyboardHandling();
		void terminateKeyboardHandling();

		static LRESULT CALLBACK childProcessCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		
	protected:
		HWND hWnd;
		HCURSOR cursor;
		hstr fpsTitle;
		bool fpsCounter;

		void _setupStyles(DWORD& style, DWORD& exstyle, bool fullscreen);
		void _adjustWindowSizeForClient(int x, int y, int& w, int& h, DWORD style, DWORD exstyle);
		void _refreshCursor();
		
		static LRESULT CALLBACK _mainProcessCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	};

}

#endif
#endif
