/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _OPENKODE_WINDOW
#ifdef __APPLE__
	#include <TargetConditionals.h>
	#if TARGET_OS_IPHONE
		#import <UIKit/UIWindow.h>
    #else
        #define MAC_OS
	#endif
#endif

#ifdef _ANDROID
#include <jni.h>
#endif

#include <KD/kd.h>

#include <hltypes/hltypesUtil.h>
#include <hltypes/hlog.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "RenderSystem.h"
#include "SystemDelegate.h"
#include "Timer.h"

#ifdef _EGL
#include "egl.h"
#endif
#include "OpenGL_RenderSystem.h"
#include "OpenKODE_Window.h"
#include "OpenKODE_Keys.h"

#if TARGET_OS_IPHONE
#import <AVFoundation/AVFoundation.h>

bool (*iOShandleUrlCallback)(chstr url) = NULL; // KD-TODO
#endif

namespace april
{
#ifdef _ANDROID
	extern JavaVM* javaVM;
#endif

#if TARGET_OS_IPHONE
	static void iosSetupAudioSession()
	{
		// kspes: copied this from iOS app delegate code, it's needed for OpenKODE and OpenAL to play along on iOS
		if ([[[UIDevice currentDevice] systemVersion] compare:@"5.0" options:NSNumericSearch] == NSOrderedAscending)
		{
			// less than iOS 5.0 - workarround for an apple bug where the audio sesion get's interrupted while using AVAssetReader and similar
			AVAudioSession *audioSession = [AVAudioSession sharedInstance];
			[audioSession setActive: NO error: nil];
			[audioSession setCategory:AVAudioSessionCategoryPlayback error:nil];
			
			// Modifying Playback Mixing Behavior, allow playing music in other apps
			UInt32 allowMixing = true;
			AudioSessionSetProperty(kAudioSessionProperty_OverrideCategoryMixWithOthers, sizeof(allowMixing), &allowMixing);
			[audioSession setActive: YES error: nil];
		}
		else
		{
			[[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryAmbient error:NULL];
		}
	}
#endif
	
	void KD_APIENTRY _processEventPause(const KDEvent* evt)
	{
		switch (evt->type)
		{
		case KD_EVENT_PAUSE:
			hlog::write(logTag, "OpenKODE pause event received.");
			if (april::window != NULL)
			{
				april::window->handleActivityChangeEvent(false);
			}
#if defined(_IOS) || defined(_ANDROID)
			if (april::rendersys != NULL)
			{
				april::rendersys->unloadTextures();
			}
#endif
			break;
		}
	}
	
	void KD_APIENTRY _processEventResume(const KDEvent* evt)
	{
		switch (evt->type)
		{
		case KD_EVENT_RESUME:
			hlog::write(logTag, "OpenKODE resume event received.");
			if (april::window != NULL)
			{
				april::window->handleActivityChangeEvent(true);
			}
			break;
		}
	}
	
	void KD_APIENTRY _processEventLowMemory(const KDEvent* evt)
	{
		switch (evt->type)
		{
		case KD_EVENT_LOWMEM:
			hlog::write(logTag, "Received libKD memory warning!");
			if (april::window != NULL)
			{
				april::window->handleLowMemoryWarning();
			}
			break;
		}
	}
	
	OpenKODE_Window::OpenKODE_Window() : Window()
	{
#if TARGET_OS_IPHONE
		iosSetupAudioSession();
#endif
		this->name = APRIL_WS_OPENKODE;
		this->kdWindow = NULL;
		memset(this->kdTouches, 0, 16 * sizeof(bool));
#if defined(_WIN32) && !defined(_EGL)
		hlog::warn(april::logTag, "OpenKODE Window requires EGL to be present!");
#endif
		initOpenKODEKeyMap();
		
		launchDelay = 0;
	}

	OpenKODE_Window::~OpenKODE_Window()
	{
		this->destroy();
	}

	hstr OpenKODE_Window::getParam(chstr param)
	{
		if (param == "launch_delay") return this->launchDelay;
		else return Window::getParam(param);
	}
	
	void OpenKODE_Window::setParam(chstr param, chstr value)
	{
		if (param == "launch_delay") this->launchDelay = value;
		else Window::setParam(param, value);
	}
	
	bool OpenKODE_Window::create(int w, int h, bool fullscreen, chstr title, Window::Options options)
	{
		if (!Window::create(w, h, fullscreen, title, options))
		{
			return false;
		}
		this->inputMode = TOUCH;
		if (w <= 0 || h <= 0)
		{
			hlog::errorf(april::logTag, "Cannot create window with size: %d x %d", w, h);
			this->destroy();
			return false;
		}
#ifdef _EGL
		this->kdWindow = kdCreateWindow(april::egl->display, april::egl->config, NULL);
#endif
		if (this->kdWindow == NULL)
		{
			hlog::error(april::logTag, "Can't create KD Window!");
			this->destroy();
			return false;
		}
		if (fullscreen) // KD only supports fullscreen in the screen's resolution
		{
			kdQueryAttribi(KD_ATTRIB_WIDTH, &w);
			kdQueryAttribi(KD_ATTRIB_HEIGHT, &h);
		}
		KDint32 size[] = {w, h};
		kdSetWindowPropertyiv(this->kdWindow, KD_WINDOWPROPERTY_SIZE, size);
		kdSetWindowPropertycv(this->kdWindow, KD_WINDOWPROPERTY_CAPTION, title.c_str());
		
		if (this->launchDelay > 0)
		{
			hlog::writef(april::logTag, "launch_delay param set to %.1f, waiting", this->launchDelay);
			hthread::sleep(this->launchDelay * 1000);
		}
#ifdef _EGL // KD doesn't actually work without EGL
		hlog::write(april::logTag, "Calling kdRealizeWindow()");
		if (kdRealizeWindow(this->kdWindow, &april::egl->hWnd) != 0)
#endif
		{
			hlog::error(april::logTag, "Can't realize KD Window!");
			this->destroy();
			return false;
		}
#ifdef _EGL
		april::egl->create();
#endif
		kdInstallCallback(&_processEventPause, KD_EVENT_PAUSE, NULL);
		kdInstallCallback(&_processEventResume, KD_EVENT_RESUME, NULL);
		kdInstallCallback(&_processEventLowMemory, KD_EVENT_LOWMEM, NULL);
		return true;
	}
	
	bool OpenKODE_Window::destroy()
	{
		if (!Window::destroy())
		{
			return false;
		}
		kdInstallCallback(NULL, KD_EVENT_PAUSE, NULL);
		kdInstallCallback(NULL, KD_EVENT_RESUME, NULL);
		kdInstallCallback(NULL, KD_EVENT_LOWMEM, NULL);
#ifdef _EGL
		april::egl->destroy();
#endif
		if (this->kdWindow != NULL)
		{
			kdDestroyWindow(this->kdWindow);
			this->kdWindow = NULL;
		}
		return true;
	}

	int OpenKODE_Window::getWidth()
	{
		KDint32 size[] = {0, 0};
		kdGetWindowPropertyiv(this->kdWindow, KD_WINDOWPROPERTY_SIZE, size);
		return size[0];
	}
	
	int OpenKODE_Window::getHeight()
	{
		KDint32 size[] = {0, 0};
		kdGetWindowPropertyiv(this->kdWindow, KD_WINDOWPROPERTY_SIZE, size);
		return size[1];
	}
	
	void OpenKODE_Window::setTitle(chstr title)
	{
		this->title = title;
		kdSetWindowPropertycv(this->kdWindow, KD_WINDOWPROPERTY_CAPTION, this->title.c_str());
	}
	
	bool OpenKODE_Window::isCursorVisible()
	{
		return (Window::isCursorVisible() || !this->isCursorInside());
	}
	
	void OpenKODE_Window::setCursorVisible(bool value)
	{
		Window::setCursorVisible(value);
#if !defined(_ANDROID) && !defined(_IOS) && !defined(_WINP8)
		if (this->kdWindow != NULL)
		{
			KDint param = value ? KD_CURSOR_ARROW : KD_CURSOR_NONE;
			kdSetWindowPropertyiv(this->kdWindow, KD_WINDOWPROPERTY_CURSOR, &param);
		}
#endif
	}
	
	void* OpenKODE_Window::getBackendId()
	{
#if TARGET_OS_IPHONE
		UIWindow* window = [UIApplication sharedApplication].keyWindow;
		if (!window)
		{
			window = [[UIApplication sharedApplication].windows objectAtIndex:0];
		}
		UIViewController* controller = [window rootViewController];
		return controller;
#elif defined(_ANDROID)
		return javaVM;
#elif defined(_EGL)
		return april::egl->hWnd;
#else
		return 0;
#endif
	}

	void OpenKODE_Window::setResolution(int w, int h, bool fullscreen)
	{
		if (fullscreen) // KD only supports fullscreen in the screen's resolution
		{
			kdQueryAttribi(KD_ATTRIB_WIDTH, &w);
			kdQueryAttribi(KD_ATTRIB_HEIGHT, &h);
		}
		this->fullscreen = fullscreen;
		KDint32 size[] = {w, h};
		kdSetWindowPropertyiv(this->kdWindow, KD_WINDOWPROPERTY_SIZE, size);
		april::rendersys->setViewport(grect(0.0f, 0.0f, (float)w, (float)h));
		if (this->systemDelegate != NULL)
		{
			this->systemDelegate->onWindowSizeChanged(w, h, fullscreen);
		}
	}

	void OpenKODE_Window::handleActivityChangeEvent(bool active)
	{
		if (this->focused != active)
		{
			this->handleFocusChangeEvent(active);
		}
	}

	bool OpenKODE_Window::updateOneFrame()
	{
#ifdef MAC_OS
		// MacOS has a bug where if you move the cursor to the dock area and back, it resets to the arrow cursor
		// in Mac window we implemented a workarround, but in OpenKODE, depending on implementation, it may not work
		// so the simplest solution would be to just reset the cursor every 60 frames and problem solved.
		static int timer = 0;
		if (!this->cursorVisible)
		{
			++timer;
			if (timer >= 60)
			{
				timer = 0;
				KDint param = KD_CURSOR_NONE;
				kdSetWindowPropertyiv(this->kdWindow, KD_WINDOWPROPERTY_CURSOR, &param);
			}
		}
#endif
		this->checkEvents();
		// rendering
		bool result = Window::updateOneFrame();
#ifndef _WINRT
		this->setTitle(this->title); // has to come after Window::updateOneFrame(), otherwise FPS value in title would be late one frame
#endif
		return result;
	}
	
	void OpenKODE_Window::presentFrame()
	{
#ifdef _EGL
		april::egl->swapBuffers();
#endif
	}
	
	int OpenKODE_Window::_getAprilTouchIndex(int kdIndex)
	{
		int index = -1;
		for_iter (i, 0, kdIndex + 1)
		{
			if (this->kdTouches[i])
			{
				++index;
			}
		}
		return index;
	}

	bool OpenKODE_Window::_processEvent(const KDEvent* evt)
	{
		int kdSize[2] = {0, 0};
		switch (evt->type)
		{
		case KD_EVENT_QUIT:
			this->handleQuitRequest(false);
			this->terminateMainLoop();
			return true;
		case KD_EVENT_WINDOW_CLOSE:
			this->handleQuitRequest(true);
			return true;
		case KD_EVENT_PAUSE:
			hlog::write(logTag, "OpenKODE pause event received.");
			this->handleActivityChangeEvent(false);
#if defined(_IOS) || defined(_ANDROID)
			if (april::rendersys != NULL)
			{
				april::rendersys->unloadTextures();
			}
#endif
			return true;
		case KD_EVENT_RESUME:
			hlog::write(logTag, "OpenKODE resume event received.");
			this->handleActivityChangeEvent(true);
			return true;
		case KD_EVENT_WINDOW_FOCUS:
			{
				bool active = (evt->data.windowfocus.focusstate != 0);
				if (this->focused != active)
				{
					this->handleFocusChangeEvent(active);
				}
			}
			return true;
		case KD_EVENT_INPUT:
			if (evt->data.input.value.i != 0)
			{
				if (evt->data.input.index < KD_IOGROUP_CHARS) // because key and char events are separate
				{
					this->queueKeyEvent(april::Window::KEY_DOWN, kd2april(evt->data.input.index), 0);
				}
				else
				{
					this->queueKeyEvent(april::Window::KEY_DOWN, april::AK_NONE, evt->data.input.index - KD_IOGROUP_CHARS);
				}
			}
			else
			{
				this->queueKeyEvent(april::Window::KEY_UP, kd2april(evt->data.input.index), 0);
			}
			return true;
		case KD_EVENT_INPUT_POINTER:
			{
				int index = evt->data.inputpointer.index;
				gvec2 pos((float)evt->data.inputpointer.x, (float)evt->data.inputpointer.y);
				if (this->_isMousePointer())
				{
					this->setInputMode(MOUSE);
					if (index == KD_INPUT_POINTER_X || index == KD_INPUT_POINTER_Y)
					{
						this->queueMouseEvent(Window::MOUSE_MOVE, pos, AK_NONE);
						this->cursorPosition = pos;
					}
					else if (index == KD_INPUT_POINTER_SELECT)
					{
						int s = evt->data.inputpointer.select;
						bool state[3];
						state[0] = ((s & 1) != 0);
						state[1] = ((s & 2) != 0);
						state[2] = ((s & 4) != 0);
						for_iter (i, 0, 3)
						{
							if (state[i] != this->kdTouches[i])
							{							
								this->queueMouseEvent(state[i] ? Window::MOUSE_DOWN : Window::MOUSE_UP, pos, (i == 0 ? AK_LBUTTON : (i == 1 ? AK_RBUTTON : AK_MBUTTON)));
							}
						}
						memcpy(this->kdTouches, state, 3 * sizeof(bool));
						this->cursorPosition = pos;
					}
					else if (index == KD_INPUT_POINTER_WHEEL)
					{
						int deltaV = -(short)(evt->data.inputpointer.select >> 16);
						int deltaH = -(short)(evt->data.inputpointer.select & 0xFFFF);
						this->queueMouseEvent(Window::MOUSE_SCROLL, gvec2(deltaH * 0.2f, deltaV * 0.2f), AK_NONE);
					}
				}
				else
				{
					this->setInputMode(TOUCH);
					int i = 0;
					int j = 0;
					int touchIndex = 0;
					for (; i < 4; ++i, j += KD_IO_POINTER_STRIDE)
					{
						if (index == KD_INPUT_POINTER_X + j || index == KD_INPUT_POINTER_Y + j)
						{
							touchIndex = this->_getAprilTouchIndex(i);
							if (touchIndex >= 0)
							{
								this->queueTouchEvent(Window::MOUSE_MOVE, pos, touchIndex);
							}
							break;
						}
						if (index == KD_INPUT_POINTER_SELECT + j)
						{
							if (evt->data.inputpointer.select != 0)
							{
								this->kdTouches[i] = true;
								touchIndex = this->_getAprilTouchIndex(i);
								if (touchIndex >= 0)
								{
									this->queueTouchEvent(Window::MOUSE_DOWN, pos, touchIndex);
								}
							}
							else
							{
								touchIndex = this->_getAprilTouchIndex(i);
								if (touchIndex >= 0)
								{
									this->queueTouchEvent(Window::MOUSE_UP, pos, touchIndex);
								}
								this->kdTouches[i] = false;
							}
							break;
						}
					}
					if (i == 0)
					{
						this->cursorPosition = pos;
					}
				}
			}
			return true;
		case KD_EVENT_WINDOWPROPERTY_CHANGE:
			if (evt->data.windowproperty.pname == KD_WINDOWPROPERTY_SIZE)
			{
				kdGetWindowPropertyiv(this->kdWindow, KD_WINDOWPROPERTY_SIZE, kdSize);
				this->_setRenderSystemResolution(kdSize[0], kdSize[1], this->fullscreen);
			}
			return true;
		}
		return false;
	}
	
	void OpenKODE_Window::checkEvents()
	{
		kdPumpEvents();
		const KDEvent* evt;
		// 1 milisecond as timeout
		while (this->running && (evt = kdWaitEvent(1000000L)) != NULL)
		{
			if (!this->_processEvent(evt))
			{
				kdDefaultEvent(evt);
			}
		}
		Window::checkEvents();
	}
	
	void OpenKODE_Window::beginKeyboardHandling()
	{
		kdKeyboardShow(this->kdWindow, 1);
#ifdef _WINRT
		if (this->inputMode == TOUCH && !this->virtualKeyboardVisible)
		{
#ifndef _WINP8
			this->handleVirtualKeyboardChangeEvent(true, 0.5f);
#else
			this->handleVirtualKeyboardChangeEvent(true, 0.53f);
#endif
		}
#endif
	}
	
	void OpenKODE_Window::terminateKeyboardHandling()
	{
		kdKeyboardShow(this->kdWindow, 0);
#ifdef _WINRT
		if (this->virtualKeyboardVisible)
		{
			this->handleVirtualKeyboardChangeEvent(false, 0.0f);
		}
#endif
	}

	bool OpenKODE_Window::_isMousePointer() 
	{ 
		KDint32 type; 
		kdStateGeti(KD_INPUT_POINTER_TYPE + 0 * KD_IO_POINTER_STRIDE, 1, &type); 
		return (type == KD_POINTER_MOUSE); 
	}

}
#endif