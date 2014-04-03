/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _ANDROID
#include <jni.h>

#include <hltypes/harray.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#define __NATIVE_INTERFACE_CLASS "net/sourceforge/april/android/NativeInterface"
#include "androidUtilJNI.h"
#include "april.h"
#include "Keys.h"
#include "main_base.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "Window.h"
#include "AndroidJNI_Keys.h"

#define PROTECTED_WINDOW_CALL(methodCall) \
	if (april::window != NULL) \
	{ \
		april::window->methodCall; \
	}
#define PROTECTED_RENDERSYS_CALL(methodCall) \
	if (april::rendersys != NULL) \
	{ \
		april::rendersys->methodCall; \
	}

namespace april
{
	extern void* javaVM;
	extern void (*dialogCallback)(MessageBoxButton);
	void (*aprilInit)(const harray<hstr>&) = NULL;
	void (*aprilDestroy)() = NULL;
	extern jobject classLoader;
	
	void JNICALL _JNI_setVariables(JNIEnv* env, jclass classe, jstring jDataPath, jstring jForcedArchivePath)
	{
		hstr dataPath = _JSTR_TO_HSTR(jDataPath);
		hstr archivePath = _JSTR_TO_HSTR(jForcedArchivePath);
		hlog::write(april::logTag, "System path: " + april::getUserDataPath());
		if (!hresource::hasZip()) // if not using APK as data file archive
		{
#ifndef _OPENKODE
			// set the resources CWD
			hresource::setCwd(dataPath + "/Android/data/" + april::getPackageName());
			hresource::setArchive(""); // not used anyway when hasZip() returns false
			hlog::write(april::logTag, "Using no-zip: " + hresource::getCwd());
#else
			hlog::write(april::logTag, "Using KD file system: " + hresource::getCwd());
#endif
		}
		else if (archivePath != "")
		{
			// using APK file as archive
			hresource::setCwd("assets");
			hresource::setArchive(archivePath);
			hlog::write(april::logTag, "Using assets: " + hresource::getArchive());
		}
		else
		{
			// using OBB file as archive
			hresource::setCwd(".");
			// using Google Play's "Expansion File" system
			hresource::setArchive(dataPath);
			hlog::write(april::logTag, "Using obb: " + hresource::getArchive());
		}
	}
	
	void JNICALL _JNI_init(JNIEnv* env, jclass classe, jobjectArray _args)
	{
		harray<hstr> args;
		int length = env->GetArrayLength(_args);
		jstring jStr;
		for_iter (i, 0, length)
		{
			args += _JSTR_TO_HSTR((jstring)env->GetObjectArrayElement(_args, i));
		}
		hlog::debug(april::logTag, "Got args:");
		foreach (hstr, it, args)
		{
			hlog::debug(april::logTag, "    " + (*it));
		}
		(*aprilInit)(args);
	}
	
	void JNICALL _JNI_destroy(JNIEnv* env, jclass classe)
	{
		(*aprilDestroy)();
	}
	
	bool JNICALL _JNI_render(JNIEnv* env, jclass classe)
	{
		if (april::window != NULL)
		{
			return april::window->updateOneFrame();
		}
		return true;
	}
	
	void JNICALL _JNI_onKeyDown(JNIEnv* env, jclass classe, jint keyCode, jint charCode)
	{
		PROTECTED_WINDOW_CALL(queueKeyEvent(april::Window::KEY_DOWN, android2april((int)keyCode), (unsigned int)charCode));
	}
	
	void JNICALL _JNI_onKeyUp(JNIEnv* env, jclass classe, jint keyCode)
	{
		PROTECTED_WINDOW_CALL(queueKeyEvent(april::Window::KEY_UP, android2april((int)keyCode), 0));
	}
	
	void JNICALL _JNI_onTouch(JNIEnv* env, jclass classe, jint type, jfloat x, jfloat y, jint index)
	{
		PROTECTED_WINDOW_CALL(queueTouchEvent((april::Window::MouseEventType)type, gvec2((float)x, (float)y), (int)index));
	}
	
	void JNICALL _JNI_onButtonDown(JNIEnv* env, jclass classe, jint keyCode, jint charCode)
	{
		PROTECTED_WINDOW_CALL(queueControllerEvent(april::Window::CONTROLLER_DOWN, (Button)(int)keyCode));
	}
	
	void JNICALL _JNI_onButtonUp(JNIEnv* env, jclass classe, jint keyCode)
	{
		PROTECTED_WINDOW_CALL(queueControllerEvent(april::Window::CONTROLLER_UP, (Button)(int)keyCode));
	}
	
	void JNICALL _JNI_onWindowFocusChanged(JNIEnv* env, jclass classe, jboolean jFocused)
	{
		bool focused = (jFocused != JNI_FALSE);
		hlog::write(april::logTag, "onWindowFocusChanged(" + hstr(focused) + ")");
		PROTECTED_WINDOW_CALL(handleFocusChangeEvent(focused));
	}
	
	void JNICALL _JNI_onVirtualKeyboardChanged(JNIEnv* env, jclass classe, jboolean jVisible, jfloat jHeightRatio)
	{
		bool visible = (jVisible != JNI_FALSE);
		float heightRatio = (float)jHeightRatio;
		hlog::write(april::logTag, "onVirtualKeyboardChanged(" + hstr(visible) + "," + hstr(heightRatio) + ")");
		PROTECTED_WINDOW_CALL(handleVirtualKeyboardChangeEvent(visible, heightRatio));
	}
	
	void JNICALL _JNI_onLowMemory(JNIEnv* env, jclass classe)
	{
		hlog::write(april::logTag, "onLowMemoryWarning()");
		PROTECTED_WINDOW_CALL(handleLowMemoryWarning());
	}
	
	void JNICALL _JNI_onSurfaceCreated(JNIEnv* env, jclass classe)
	{
		hlog::write(april::logTag, "Android View::onSurfaceCreated()");
		PROTECTED_RENDERSYS_CALL(reset());
	}
	
	void JNICALL _JNI_activityOnCreate(JNIEnv* env, jclass classe)
	{
		hlog::write(april::logTag, "Android Activity::onCreate()");
	}
	
	void JNICALL _JNI_activityOnStart(JNIEnv* env, jclass classe)
	{
		hlog::write(april::logTag, "Android Activity::onStart()");
	}
	
	void JNICALL _JNI_activityOnResume(JNIEnv* env, jclass classe)
	{
		hlog::write(april::logTag, "Android Activity::onResume()");
		PROTECTED_WINDOW_CALL(handleActivityChangeEvent(true));
	}
	
	void JNICALL _JNI_activityOnPause(JNIEnv* env, jclass classe)
	{
		hlog::write(april::logTag, "Android Activity::onPause()");
		PROTECTED_WINDOW_CALL(handleActivityChangeEvent(false));
		PROTECTED_RENDERSYS_CALL(unloadTextures());
	}
	
	void JNICALL _JNI_activityOnStop(JNIEnv* env, jclass classe)
	{
		hlog::write(april::logTag, "Android Activity::onStop()");
	}
	
	void JNICALL _JNI_activityOnDestroy(JNIEnv* env, jclass classe)
	{
		hlog::write(april::logTag, "Android Activity::onDestroy()");
		if (april::classLoader != NULL)
		{
			env->DeleteGlobalRef(april::classLoader);
			april::classLoader = NULL;
		}
	}
	
	void JNICALL _JNI_activityOnRestart(JNIEnv* env, jclass classe)
	{
		hlog::write(april::logTag, "Android Activity::onRestart()");
	}
	
	void JNICALL _JNI_onDialogOk(JNIEnv* env, jclass classe)
	{
		if (dialogCallback != NULL)
		{
			(*dialogCallback)(MESSAGE_BUTTON_OK);
		}
	}
	
	void JNICALL _JNI_onDialogYes(JNIEnv* env, jclass classe)
	{
		if (dialogCallback != NULL)
		{
			(*dialogCallback)(MESSAGE_BUTTON_YES);
		}
	}
	
	void JNICALL _JNI_onDialogNo(JNIEnv* env, jclass classe)
	{
		if (dialogCallback != NULL)
		{
			(*dialogCallback)(MESSAGE_BUTTON_NO);
		}
	}
	
	void JNICALL _JNI_onDialogCancel(JNIEnv* env, jclass classe)
	{
		if (dialogCallback != NULL)
		{
			(*dialogCallback)(MESSAGE_BUTTON_CANCEL);
		}
	}
	
#define METHOD_COUNT 24 // make sure this fits
	static JNINativeMethod methods[METHOD_COUNT] =
	{
		{"setVariables",						_JARGS(_JVOID, _JSTR _JSTR),					(void*)&april::_JNI_setVariables						},
		{"init",								_JARGS(_JVOID, _JARR(_JSTR)),					(void*)&april::_JNI_init								},
		{"destroy",								_JARGS(_JVOID, ),								(void*)&april::_JNI_destroy								},
		{"render",								_JARGS(_JBOOL, ),								(void*)&april::_JNI_render								},
		{"onKeyDown",							_JARGS(_JVOID, _JINT _JINT),					(bool*)&april::_JNI_onKeyDown							},
		{"onKeyUp",								_JARGS(_JVOID, _JINT),							(bool*)&april::_JNI_onKeyUp								},
		{"onTouch",								_JARGS(_JVOID, _JINT _JFLOAT _JFLOAT _JINT),	(void*)&april::_JNI_onTouch								},
		{"onButtonDown",						_JARGS(_JVOID, _JINT),							(bool*)&april::_JNI_onButtonDown						},
		{"onButtonUp",							_JARGS(_JVOID, _JINT),							(bool*)&april::_JNI_onButtonUp							},
		{"onWindowFocusChanged",				_JARGS(_JVOID, _JBOOL),							(void*)&april::_JNI_onWindowFocusChanged				},
		{"onVirtualKeyboardChanged",			_JARGS(_JVOID, _JBOOL _JFLOAT),					(void*)&april::_JNI_onVirtualKeyboardChanged			},
		{"onLowMemory",							_JARGS(_JVOID, ),								(void*)&april::_JNI_onLowMemory							},
		{"onSurfaceCreated",					_JARGS(_JVOID, ),								(void*)&april::_JNI_onSurfaceCreated					},
		{"activityOnCreate",					_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnCreate					},
		{"activityOnStart",						_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnStart						},
		{"activityOnResume",					_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnResume					},
		{"activityOnPause",						_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnPause						},
		{"activityOnStop",						_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnStop						},
		{"activityOnDestroy",					_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnDestroy					},
		{"activityOnRestart",					_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnRestart					},
		{"onDialogOk",							_JARGS(_JVOID, ),								(void*)&april::_JNI_onDialogOk							},
		{"onDialogYes",							_JARGS(_JVOID, ),								(void*)&april::_JNI_onDialogYes							},
		{"onDialogNo",							_JARGS(_JVOID, ),								(void*)&april::_JNI_onDialogNo							},
		{"onDialogCancel",						_JARGS(_JVOID, ),								(void*)&april::_JNI_onDialogCancel						}
	};
	
	jint JNI_OnLoad(void (*anAprilInit)(const harray<hstr>&), void (*anAprilDestroy)(), JavaVM* vm, void* reserved)
	{
		april::javaVM = (void*)vm;
		april::aprilInit = anAprilInit;
		april::aprilDestroy = anAprilDestroy;
		JNIEnv* env = NULL;
		if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
		{
			return -1;
		}
		jclass classNativeInterface = env->FindClass(__NATIVE_INTERFACE_CLASS);
		if (env->RegisterNatives(classNativeInterface, methods, METHOD_COUNT) != 0)
		{
			return -1;
		}
#ifdef _OPENKODE // not really needed when OpenKODE isn't used
		jclass classClass = env->FindClass("java/lang/Class");
		jmethodID methodGetClassLoader = env->GetMethodID(classClass, "getClassLoader", _JARGS(_JCLASS("java/lang/ClassLoader"), ));
		jobject classLoader = env->CallObjectMethod(classNativeInterface, methodGetClassLoader);
		april::classLoader = env->NewGlobalRef(classLoader);
#endif
		return JNI_VERSION_1_6;
	}

}

#endif
