/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _ANDROIDJNI_WINDOW
#include <jni.h>

#include <gtypes/Vector2.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hthread.h>

#define __NATIVE_INTERFACE_CLASS "net/sourceforge/april/android/NativeInterface"
#include "androidUtilJNI.h"
#include "AndroidJNI_Window.h"
#include "AndroidJNI_Keys.h"
#include "april.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "SystemDelegate.h"
#include "Timer.h"

namespace april
{
	extern JavaVM* javaVM;

	AndroidJNI_Window::AndroidJNI_Window() : Window(), width(0), height(0),
		forcedFocus(false)
	{
		this->name = APRIL_WS_ANDROIDJNI;
		initAndroidKeyMap();
	}

	AndroidJNI_Window::~AndroidJNI_Window()
	{
		this->destroy();
	}

	bool AndroidJNI_Window::create(int w, int h, bool fullscreen, chstr title, Window::Options options)
	{
		if (!Window::create(w, h, true, title, options))
		{
			return false;
		}
		this->width = w;
		this->height = h;
		this->inputMode = TOUCH;
		this->forcedFocus = false;
		return true;
	}
	
	void* AndroidJNI_Window::getBackendId()
	{
		return javaVM;
	}

	void AndroidJNI_Window::enterMainLoop()
	{
		hlog::error(april::logTag, "Using enterMainLoop on Android JNI!");
		exit(-1);
	}
	
	void AndroidJNI_Window::presentFrame()
	{
		APRIL_GET_NATIVE_INTERFACE_METHOD(classNativeInterface, methodSwapBuffers, "swapBuffers", _JARGS(_JVOID, ));
		env->CallStaticVoidMethod(classNativeInterface, methodSwapBuffers);
	}

	void AndroidJNI_Window::queueTouchEvent(Window::MouseEventType type, gvec2 position, int index)
	{
		if (type == MOUSE_DOWN || type == MOUSE_UP)
		{
			this->setInputMode(TOUCH);
		}
		Window::queueTouchEvent(type, position, index);
	}

	void AndroidJNI_Window::queueControllerEvent(Window::ControllerEventType type, Button buttonCode)
	{
		this->setInputMode(CONTROLLER);
		Window::queueControllerEvent(type, buttonCode);
	}

	void AndroidJNI_Window::beginKeyboardHandling()
	{
		APRIL_GET_NATIVE_INTERFACE_METHOD(classNativeInterface, methodShowVirtualKeyboard, "showVirtualKeyboard", _JARGS(_JVOID, ));
		env->CallStaticVoidMethod(classNativeInterface, methodShowVirtualKeyboard);
	}
	
	void AndroidJNI_Window::terminateKeyboardHandling()
	{
		APRIL_GET_NATIVE_INTERFACE_METHOD(classNativeInterface, methodHideVirtualKeyboard, "hideVirtualKeyboard", _JARGS(_JVOID, ));
		env->CallStaticVoidMethod(classNativeInterface, methodHideVirtualKeyboard);
	}

	void AndroidJNI_Window::handleActivityChangeEvent(bool active)
	{
		if (!active)
		{
			if (this->focused)
			{
				this->forcedFocus = true;
				Window::handleFocusChangeEvent(false);
			}
		}
		else if (this->forcedFocus)
		{
			// only return focus if previously lost focus through acitvity state change
			this->forcedFocus = false;
			Window::handleFocusChangeEvent(true);
		}
	}

	void AndroidJNI_Window::handleFocusChangeEvent(bool focused)
	{
		this->forcedFocus = false;
		Window::handleFocusChangeEvent(focused);
	}

}
#endif