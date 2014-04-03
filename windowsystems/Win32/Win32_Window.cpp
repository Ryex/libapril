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

#ifdef _WIN32_WINDOW
#include <hltypes/hltypesUtil.h>
#include <hltypes/hlog.h>
#include <hltypes/hplatform.h>
#include <hltypes/hresource.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "SystemDelegate.h"
#include "Timer.h"
#include "Win32_Window.h"

#ifdef _OPENGL
#include "OpenGL_RenderSystem.h"
#endif
#ifdef _EGL
#include "egl.h"
#endif

#define APRIL_WIN32_WINDOW_CLASS L"AprilWin32Window"
#define STYLE_FULLSCREEN WS_POPUP
#define STYLE_WINDOWED (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX)
#define EXSTYLE_FULLSCREEN WS_EX_TOPMOST
#define EXSTYLE_WINDOWED WS_EX_LEFT

namespace april
{
	Win32_Window::Win32_Window() : Window()
	{
		this->name = APRIL_WS_WIN32;
		this->hWnd = NULL;
		this->cursorExtensions += ".ani";
		this->cursorExtensions += ".cur";
	}

	Win32_Window::~Win32_Window()
	{
		this->destroy();
	}

	// Considering that Iron Man and Batman's only real superpower is being
	// super rich and smart, Bill Gates turned out to be a real disappointment.
	bool Win32_Window::create(int w, int h, bool fullscreen, chstr title, Window::Options options)
	{
		if (!Window::create(w, h, fullscreen, title, options))
		{
			return false;
		}
		this->inputMode = MOUSE;
		this->cursor = NULL;
		// Win32
		WNDCLASSEXW wc;
		memset(&wc, 0, sizeof(WNDCLASSEX));
		HINSTANCE hinst = GetModuleHandle(0);
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc = &Win32_Window::_mainProcessCallback;
		wc.hInstance = hinst;
		wc.hCursor = LoadCursor(0, IDC_ARROW);
		wc.lpszClassName = APRIL_WIN32_WINDOW_CLASS;
		wc.hIcon = (HICON)LoadImage(hinst, MAKEINTRESOURCE(1), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		RegisterClassExW(&wc);
		// determine position
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);
		int x = 0;
		int y = 0;
		if (!this->fullscreen)
		{
			x = (screenWidth - w) / 2;
			y = (screenHeight - h) / 2;
		}
		// setting the necessary styles
		DWORD style = 0;
		DWORD exstyle = 0;
		this->_setupStyles(style, exstyle, this->fullscreen);
		if (!this->fullscreen)
		{
			this->_adjustWindowSizeForClient(x, y, w, h, style, exstyle);
			x = (screenWidth - w) / 2;
			y = (screenHeight - h) / 2;
		}
		// create window
		this->hWnd = CreateWindowExW(exstyle, APRIL_WIN32_WINDOW_CLASS, this->title.w_str().c_str(), style, x, y, w, h, NULL, NULL, hinst, NULL);
		// display the window on the screen
		ShowWindow(this->hWnd, SW_SHOWNORMAL);
		UpdateWindow(this->hWnd);
		SetCursor(wc.hCursor);
		this->setCursorVisible(true);
		this->fpsCounter = options.fpsCounter;
#ifdef _EGL
		april::egl->create();
#endif
		return true;
	}
	
	bool Win32_Window::destroy()
	{
		if (!Window::destroy())
		{
			return false;
		}
		if (this->hWnd != 0)
		{
			DestroyWindow(this->hWnd);
			UnregisterClassW(APRIL_WIN32_WINDOW_CLASS, GetModuleHandle(0));
			this->hWnd = 0;
		}
		this->cursor = NULL;
#ifdef _EGL
		april::egl->destroy();
#endif
		return true;
	}

	void Win32_Window::setTitle(chstr title)
	{
		if (this->fpsCounter)
		{
			hstr t = title + hsprintf(" [FPS: %d]", this->fps);
			// optimization to prevent setting title every frame
			if (t == this->fpsTitle) return;
			this->fpsTitle = t;
			SetWindowTextW(this->hWnd, t.w_str().c_str());
		}
		else
		{
			SetWindowTextW(this->hWnd, title.w_str().c_str());
		}
		this->title = title;
	}
	
	bool Win32_Window::isCursorVisible()
	{
		return (Window::isCursorVisible() || !this->isCursorInside());
	}
	
	void Win32_Window::setCursorVisible(bool value)
	{
		Window::setCursorVisible(value);
		this->_refreshCursor();
	}
	
	void Win32_Window::setCursorFilename(chstr value)
	{
		Window::setCursorFilename(value);
		this->_refreshCursor();
	}

	void Win32_Window::_refreshCursor()
	{
		HCURSOR cursor = NULL;
		if (this->isCursorVisible())
		{
			hstr filename = this->_findCursorFile();
			if (filename != "")
			{
				this->cursor = LoadCursorFromFileW(filename.w_str().c_str());
				cursor = this->cursor;
			}
			else
			{
				this->cursor = NULL;
				cursor = LoadCursorW(0, IDC_ARROW);
			}
		}
		SetCursor(cursor);
	}

	int Win32_Window::getWidth()
	{
		RECT rect;
		GetClientRect(this->hWnd, &rect);
		return (rect.right - rect.left);
	}
	
	int Win32_Window::getHeight()
	{
		RECT rect;
		GetClientRect(this->hWnd, &rect);
		return (rect.bottom - rect.top);
	}

	void* Win32_Window::getBackendId()
	{
		return this->hWnd;
	}

	void Win32_Window::setResolution(int w, int h, bool fullscreen)
	{
		if (this->fullscreen == fullscreen && this->getWidth() == w && this->getHeight() == h)
		{
			return;
		}
		// do NOT change the order the following function calls or else dragons
		// setting the necessary styles
		DWORD style = 0;
		DWORD exstyle = 0;
		this->_setupStyles(style, exstyle, fullscreen);
		SetWindowLongPtr(this->hWnd, GWL_EXSTYLE, exstyle);
		SetWindowLongPtr(this->hWnd, GWL_STYLE, style);
		// changing display settings
		if (fullscreen)
		{
			DEVMODE deviceMode;
			deviceMode.dmPelsWidth = w;
			deviceMode.dmPelsWidth = h;
			deviceMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
			ChangeDisplaySettings(&deviceMode, CDS_FULLSCREEN);
		}
		if (this->fullscreen || fullscreen)
		{
			ChangeDisplaySettings(NULL, CDS_RESET);
		}
		// window positions
		int x = 0;
		int y = 0;
		if (!fullscreen)
		{
			if (this->fullscreen)
			{
				x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
				y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;
			}
			else
			{
				RECT rect;
				GetWindowRect(this->hWnd, &rect);
				x = rect.left;
				y = rect.top;
			}
		}
		// updating/executing all remaining changes
		if (!fullscreen)
		{
			this->_adjustWindowSizeForClient(x, y, w, h, style, exstyle);
		}
		SetWindowPos(this->hWnd, (fullscreen ? HWND_TOPMOST : HWND_NOTOPMOST), x, y, w, h, 0);
		ShowWindow(this->hWnd, SW_SHOW);
		UpdateWindow(this->hWnd);
		this->fullscreen = fullscreen;
		this->_setRenderSystemResolution(this->getWidth(), this->getHeight(), this->fullscreen);
	}
	
	bool Win32_Window::updateOneFrame()
	{
		POINT w32_cursorPosition;
		// mouse position
		GetCursorPos(&w32_cursorPosition);
		ScreenToClient(this->hWnd, &w32_cursorPosition);
		this->cursorPosition.set((float)w32_cursorPosition.x, (float)w32_cursorPosition.y);
		this->checkEvents();
		// rendering
		bool result = Window::updateOneFrame();
		if (this->fpsCounter)
		{
			this->setTitle(this->title); // has to come after Window::updateOneFrame(), otherwise FPS value in title would be late one frame
		}
		return result;
	}
	
	void Win32_Window::presentFrame()
	{
#ifdef _OPENGL
		harray<hstr> renderSystems;
		renderSystems += APRIL_RS_OPENGL1;
		renderSystems += APRIL_RS_OPENGLES1;
		renderSystems += APRIL_RS_OPENGLES2;
		if (renderSystems.contains(april::rendersys->getName()))
		{
			SwapBuffers(((OpenGL_RenderSystem*)april::rendersys)->getHDC());
		}
#endif
	}
	
	void Win32_Window::checkEvents()
	{
		MSG msg;
		if (PeekMessageW(&msg, this->hWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		Window::checkEvents();
	}

	void Win32_Window::_setupStyles(DWORD& style, DWORD& exstyle, bool fullscreen)
	{
		style = STYLE_WINDOWED;
		if (fullscreen)
		{
			style = STYLE_FULLSCREEN;
		}
		else if (this->options.resizable)
		{
			style |= WS_SIZEBOX | WS_MAXIMIZEBOX;
		}
		exstyle = (fullscreen ? EXSTYLE_FULLSCREEN : EXSTYLE_WINDOWED);
	}

	void Win32_Window::_adjustWindowSizeForClient(int x, int y, int& w, int& h, DWORD style, DWORD exstyle)
	{
		RECT rect;
		rect.left = x;
		rect.top = y;
		rect.right = x + w;
		rect.bottom = y + h;
		AdjustWindowRectEx(&rect, style, FALSE, exstyle);
		w = rect.right - rect.left;
		h = rect.bottom - rect.top;
	}

	void Win32_Window::beginKeyboardHandling()
	{
		//this->handleVirtualKeyboardChangeEvent(true, 0.5f); // usually only used for testing
	}

	void Win32_Window::terminateKeyboardHandling()
	{
		//this->handleVirtualKeyboardChangeEvent(false, 0.0f); // usually only used for testing
	}

	LRESULT CALLBACK Win32_Window::_mainProcessCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (!april::window->isCreated()) // don't run callback processing if window was "destroyed"
		{
			return 1;
		}
		static bool _sizeChanging = false;
		static bool _initialSize = true;
		switch (message)
		{
		case WM_ENTERSIZEMOVE:
			_sizeChanging = true;
			break;
		case WM_EXITSIZEMOVE:
			_sizeChanging = false;
			break;
		case WM_SIZE:
			if (!april::window->isFullscreen() &&
				(_sizeChanging || wParam == SIZE_MAXIMIZED || wParam == SIZE_RESTORED && !_initialSize))
			{
				((Win32_Window*)april::window)->_setRenderSystemResolution();
				UpdateWindow(hWnd);
				april::window->performUpdate(0.0f);
				april::rendersys->presentFrame();
			}
			_initialSize = false;
			break;
		}
		return Win32_Window::childProcessCallback(hWnd, message, wParam, lParam);
	}

	LRESULT CALLBACK Win32_Window::childProcessCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (!april::window->isCreated()) // don't run callback processing if window was "destroyed"
		{
			return 1;
		}
		static bool _touchDown = false;
		static bool _doubleTapDown = false;
		static int _mouseMoveMessagesCount = 0;
		static float _wheelDelta = 0.0f;
		static bool _altKeyDown = false;
		switch (message)
		{
		case 0x0119: // WM_GESTURE (Win7+ only)
			if (wParam == 1) // GID_BEGIN
			{
				_touchDown = true;
			}
			else if (wParam == 2) // GID_END
			{
				if (_doubleTapDown)
				{ 
					_doubleTapDown = false;
					april::window->queueMouseEvent(MOUSE_UP, april::window->getCursorPosition(), AK_DOUBLETAP);
				}
				_touchDown = false;
			}
			else if (wParam == 6) // GID_TWOFINGERTAP
			{
				_doubleTapDown = true;
				april::window->queueMouseEvent(MOUSE_DOWN, april::window->getCursorPosition(), AK_DOUBLETAP);
			}
			break;
		case 0x011A: // WM_GESTURENOTIFY (win7+ only)
			_touchDown = true;
			april::window->setInputMode(april::Window::TOUCH);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			april::window->terminateMainLoop();
			break;
		case WM_CLOSE:
			if (april::window->handleQuitRequest(true))
			{
				PostQuitMessage(0);
				april::window->terminateMainLoop();
			}
			return 0;
		case WM_SYSKEYDOWN:
			if (wParam == VK_MENU)
			{
				_altKeyDown = true;
			}
			// no break here, because this is still an input message that needs to be processed normally
		case WM_KEYDOWN:
			if (_altKeyDown && wParam == VK_F4)
			{
				if (april::window->handleQuitRequest(true))
				{
					PostQuitMessage(0);
					april::window->terminateMainLoop();
				}
			}
			else
			{
				april::window->queueKeyEvent(KEY_DOWN, (april::Key)wParam, 0);
			}
			return 0;
		case WM_SYSKEYUP:
			if (wParam == VK_MENU)
			{
				_altKeyDown = false;
			}
			// no break here, because this is still an input message that needs to be processed normally
		case WM_KEYUP:
			april::window->queueKeyEvent(KEY_UP, (april::Key)wParam, 0);
			return 0;
		case WM_CHAR:
			april::window->queueKeyEvent(KEY_DOWN, AK_NONE, wParam);
			break;
		case WM_LBUTTONDOWN:
			_touchDown = true;
			_mouseMoveMessagesCount = 0;
			april::window->queueMouseEvent(MOUSE_DOWN, april::window->getCursorPosition(), AK_LBUTTON);
			if (!april::window->isFullscreen())
			{
				SetCapture((HWND)april::window->getBackendId());
			}
			break;
		case WM_RBUTTONDOWN:
			_touchDown = true;
			_mouseMoveMessagesCount = 0;
			april::window->queueMouseEvent(MOUSE_DOWN, april::window->getCursorPosition(), AK_RBUTTON);
			if (!april::window->isFullscreen())
			{
				SetCapture((HWND)april::window->getBackendId());
			}
			break;
		case WM_LBUTTONUP:
			_touchDown = false;
			april::window->queueMouseEvent(MOUSE_UP, april::window->getCursorPosition(), AK_LBUTTON);
			if (!april::window->isFullscreen())
			{
				ReleaseCapture();
			}
			break;
		case WM_RBUTTONUP:
			_touchDown = false;
			april::window->queueMouseEvent(MOUSE_UP, april::window->getCursorPosition(), AK_RBUTTON);
			if (!april::window->isFullscreen())
			{
				ReleaseCapture();
			}
			break;
		case WM_MOUSEMOVE:
			if (!_touchDown)
			{
				if (_mouseMoveMessagesCount >= 10)
				{
					april::window->setInputMode(april::Window::MOUSE);
				}
				else
				{
					++_mouseMoveMessagesCount;
				}
			}
			else
			{
				_mouseMoveMessagesCount = 0;
			}
			april::window->queueMouseEvent(MOUSE_MOVE, april::window->getCursorPosition(), AK_NONE);
			break;
		case WM_MOUSEWHEEL:
			_wheelDelta = (float)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
			if ((GET_KEYSTATE_WPARAM(wParam) & MK_CONTROL) != MK_CONTROL)
			{
				april::window->queueMouseEvent(MOUSE_SCROLL, gvec2(0.0f, -(float)_wheelDelta), AK_NONE);
			}
			else
			{
				april::window->queueMouseEvent(MOUSE_SCROLL, gvec2(-(float)_wheelDelta, 0.0f), AK_NONE);
			}
			break;
		case WM_MOUSEHWHEEL:
			_wheelDelta = (float)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
			if ((GET_KEYSTATE_WPARAM(wParam) & MK_CONTROL) != MK_CONTROL)
			{
				april::window->queueMouseEvent(MOUSE_SCROLL, gvec2(-(float)_wheelDelta, 0.0f), AK_NONE);
			}
			else
			{
				april::window->queueMouseEvent(MOUSE_SCROLL, gvec2(0.0f, -(float)_wheelDelta), AK_NONE);
			}
			break;
		case WM_SETCURSOR:
			if (!april::window->isCursorVisible())
			{
				SetCursor(NULL);
				return 1;
			}
			if (april::window->isCursorInside())
			{
				HCURSOR cursor = ((Win32_Window*)april::window)->cursor;
				if (cursor != GetCursor())
				{
					SetCursor(cursor);
				}
				if (cursor != NULL)
				{
					return 1;
				}
			}
			break;
		case WM_ACTIVATE:
			if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)
			{
				april::window->handleFocusChangeEvent(true);
			}
			else
			{
				april::window->handleFocusChangeEvent(false);
			}
			break;
		}
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}

}
#endif
