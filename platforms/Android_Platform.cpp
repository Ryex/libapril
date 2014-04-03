/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if defined(_ANDROID) && !defined(_OPENKODE)
#include <jni.h>
#include <unistd.h>

#include <gtypes/Vector2.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>
#include <hltypes/hlog.h>

#define __NATIVE_INTERFACE_CLASS "net/sourceforge/april/android/NativeInterface"
#include "androidUtilJNI.h"
#include "Platform.h"
#include "RenderSystem.h"

namespace april
{
	extern SystemInfo info;
	extern void (*dialogCallback)(MessageBoxButton);
	
	SystemInfo getSystemInfo()
	{
		if (info.locale == "")
		{
			info.name = "android";
			APRIL_GET_NATIVE_INTERFACE_CLASS(classNativeInterface);
			// CPU cores
			info.cpuCores = sysconf(_SC_NPROCESSORS_CONF);
			// RAM
			info.ram = (int)(((int64_t)sysconf(_SC_PAGESIZE) * sysconf(_SC_PHYS_PAGES)) / (1024 * 1024)); // in MB
			// display resolution
			// TODO - maybe use direct Unix calls?
			jmethodID methodGetDisplayResolution = env->GetStaticMethodID(classNativeInterface, "getDisplayResolution", _JARGS(_JOBJ, ));
			jintArray jResolution = (jintArray)env->CallStaticObjectMethod(classNativeInterface, methodGetDisplayResolution);
			jint dimensions[2];
			env->GetIntArrayRegion(jResolution, 0, 2, dimensions);
			info.displayResolution.set(hroundf(dimensions[0]), hroundf(dimensions[1]));
			// display DPI
			// TODO - maybe use direct Unix calls?
			jmethodID methodGetDisplayDpi = env->GetStaticMethodID(classNativeInterface, "getDisplayDpi", _JARGS(_JINT, ));
			info.displayDpi = (int)env->CallStaticIntMethod(classNativeInterface, methodGetDisplayDpi);
			// locale
			// TODO - maybe use direct Unix calls?
			jmethodID methodGetLocale = env->GetStaticMethodID(classNativeInterface, "getLocale", _JARGS(_JSTR, ));
			info.locale = _JSTR_TO_HSTR((jstring)env->CallStaticObjectMethod(classNativeInterface, methodGetLocale));
			// OS version
			jmethodID methodGetOsVersion = env->GetStaticMethodID(classNativeInterface, "getOsVersion", _JARGS(_JSTR, ));
			harray<hstr> osVersions = _JSTR_TO_HSTR((jstring)env->CallStaticObjectMethod(classNativeInterface, methodGetOsVersion)).split('.');
			hstr majorVersion = osVersions.remove_first();
			hstr minorVersion = osVersions.join("");
			osVersions.clear();
			osVersions += majorVersion;
			osVersions += minorVersion;
			info.osVersion = (float)osVersions.join('.');
		}
		return info;
	}
	
	hstr getPackageName()
	{
		static hstr package;
		if (package == "")
		{
			APRIL_GET_NATIVE_INTERFACE_FIELD(classNativeInterface, fieldPackageName, "PackageName", _JSTR);
			package = _JSTR_TO_HSTR((jstring)env->GetStaticObjectField(classNativeInterface, fieldPackageName));
		}
		return package;
	}
	
	hstr getUserDataPath()
	{
		static hstr path;
		if (path == "")
		{
			APRIL_GET_NATIVE_INTERFACE_METHOD(classNativeInterface, methodGetUserDataPath, "getUserDataPath", _JARGS(_JSTR, ));
			path = _JSTR_TO_HSTR((jstring)env->CallStaticObjectMethod(classNativeInterface, methodGetUserDataPath));
		}
		return path;
	}
	
	void messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style,
		hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
		APRIL_GET_NATIVE_INTERFACE_METHOD(classNativeInterface, methodShowMessageBox, "showMessageBox", _JARGS(_JVOID, _JSTR _JSTR _JSTR _JSTR _JSTR _JSTR _JINT));
		// determine ok/yes/no/cancel texts
		hstr ok;
		hstr yes;
		hstr no;
		hstr cancel;
		_makeButtonLabels(&ok, &yes, &no, &cancel, buttonMask, customButtonTitles);
		// create Java strings from hstr
		jstring jTitle = (title != "" ? env->NewStringUTF(title.c_str()) : NULL);
		jstring jText = (text != "" ? env->NewStringUTF(text.c_str()) : NULL);
		jstring jOk = (ok != "" ? env->NewStringUTF(ok.c_str()) : NULL);
		jstring jYes = (yes != "" ? env->NewStringUTF(yes.c_str()) : NULL);
		jstring jNo = (no != "" ? env->NewStringUTF(no.c_str()) : NULL);
		jstring jCancel = (cancel != "" ? env->NewStringUTF(cancel.c_str()) : NULL);
		jint jIconId = 0;
		if ((style & MESSAGE_STYLE_INFO) || (style & MESSAGE_STYLE_QUESTION))
		{
			jIconId = 1;
		}
		else if ((style & MESSAGE_STYLE_WARNING) || (style & MESSAGE_STYLE_CRITICAL))
		{
			jIconId = 2;
		}
		april::dialogCallback = callback;
		// call Java AprilJNI
		env->CallStaticVoidMethod(classNativeInterface, methodShowMessageBox, jTitle, jText, jOk, jYes, jNo, jCancel, jIconId);
	}

}
#endif
