/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _OPENKODE
#include <KD/kd.h>

#include <gtypes/Vector2.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "april.h"

#ifdef _EGL
#include "egl.h"
#endif
#include "Platform.h"
#include "RenderSystem.h"
#include "Window.h"

#ifdef __APPLE__
	#include <TargetConditionals.h>
	#if TARGET_OS_IPHONE
		void getStaticiOSInfo(chstr name, april::SystemInfo& info);
	#elif TARGET_OS_MAC
		#include <sys/sysctl.h>
	#endif
#endif

#ifdef _ANDROID
#include <jni.h>
#define __NATIVE_INTERFACE_CLASS "net/sourceforge/april/android/NativeInterface"
#include "androidUtilJNI.h"
#include <unistd.h>
#endif

#ifdef _ANDROID
	#define debug_log(s) hlog::write(logTag, s)
#else
	#define debug_log(s) 
#endif

namespace april
{
	extern SystemInfo info;

	SystemInfo getSystemInfo()
	{
		if (info.locale == "")
		{
			debug_log("Fetching OpenKODE system info");
			// number of CPU cores
			info.cpuCores = 1;
			// display resolution
			int width = 0;
			int height = 0;
			
			debug_log("getting screen info");
			kdQueryAttribi(KD_ATTRIB_WIDTH, (KDint*)&width);
			kdQueryAttribi(KD_ATTRIB_HEIGHT, (KDint*)&height);
			info.displayResolution.set((float)hmax(width, height), (float)hmin(width, height));
			// display DPI
			kdQueryAttribi(KD_ATTRIB_DPI, (KDint*)&info.displayDpi);
#if TARGET_OS_IPHONE // On iOS, april prefers to use hardcoded device info than OpenKODE's info, it's more accurate
			hstr model = kdQueryAttribcv(KD_ATTRIB_PLATFORM);
			if (model.contains("(") && model.contains(")"))
			{
				hstr a, b;
				model.split("(",a, b);
				b.split(")", model, a);
				getStaticiOSInfo(model, info);
			}
#elif defined(__APPLE__) && defined(_PC_INPUT) // mac
			info.cpuCores = sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(_WINRT)
			SYSTEM_INFO w32info;
			GetNativeSystemInfo(&w32info);
			info.cpuCores = w32info.dwNumberOfProcessors;
#elif defined(_ANDROID)
			debug_log("getting java stuff");
			APRIL_GET_NATIVE_INTERFACE_CLASS(classNativeInterface);
			// CPU cores
			debug_log("getting cpu cores");
			info.cpuCores = sysconf(_SC_NPROCESSORS_CONF);
			// OS version
			debug_log("getting os version");
			jmethodID methodGetOsVersion = env->GetStaticMethodID(classNativeInterface, "getOsVersion", _JARGS(_JSTR, ));
			harray<hstr> osVersions = _JSTR_TO_HSTR((jstring)env->CallStaticObjectMethod(classNativeInterface, methodGetOsVersion)).split('.');
//			use this block for debugging if this starts crashing again
//			jobject obj = env->CallStaticObjectMethod(classNativeInterface, methodGetOsVersion);
//			debug_log(hsprintf("getting os version - 2: %p", obj));
//			jstring jstr = (jstring) obj;
//			hstr str = _JSTR_TO_HSTR(jstr);
//			debug_log(hsprintf("getting os version - 3: %s", str.c_str()));
//			harray<hstr> osVersions = str.split('.');
	
			hstr majorVersion = osVersions.remove_first();
			hstr minorVersion = osVersions.join("");
			osVersions.clear();
			osVersions += majorVersion;
			osVersions += minorVersion;
			info.osVersion = (float)osVersions.join('.');
#endif
			// RAM size
#if TARGET_IPHONE_SIMULATOR
			info.ram = 1024;
#elif defined(__APPLE__) && defined(_PC_INPUT) // mac
			int mib [] = {CTL_HW, HW_MEMSIZE};
			int64_t value = 0;
			size_t length = sizeof(value);
			if (sysctl(mib, 2, &value, &length, NULL, 0) != -1)
			{
				info.ram = value / (1024 * 1024);
			}
			else
			{
				info.ram = 2048;
			}
#else
			int pageSize;
			int pageCount;
			kdQueryAttribi(KD_ATTRIB_PAGESIZE, (KDint*)&pageSize);
			kdQueryAttribi(KD_ATTRIB_NUMPAGES, (KDint*)&pageCount);
#ifdef _WINRT
			info.ram = (int)((int64)pageSize * pageCount / (1024 * 1024)); // in MB
#else
			info.ram = (int)((int64_t)pageSize * pageCount / (1024 * 1024)); // in MB
#endif
#endif
			// other
			debug_log("getting locale");

			info.locale = hstr(kdGetLocale());
			if (info.locale == "")
			{
				info.locale = "en"; // default is "en"
			}
			else if (info.locale == "pt_PT")
			{
				info.locale = "pt-PT";
			}
			else if (info.locale.utf8_size() > 2 && info.locale != "pt-PT")
			{
				info.locale = info.locale.utf8_substr(0, 2);
			}
		}
		return info;
	}

	hstr getPackageName()
	{
#ifndef _WINRT
		return hstr(kdGetenv("KD_APP_ID"));
#else
		return _HL_PSTR_TO_HSTR(Windows::ApplicationModel::Package::Current->Id->FamilyName);
#endif
	}

	hstr getUserDataPath()
	{
		return "data";
	}
	
	void messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style,
		hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
		hstr ok;
		hstr yes;
		hstr no;
		hstr cancel;
		_makeButtonLabels(&ok, &yes, &no, &cancel, buttonMask, customButtonTitles);
		const char* buttons[4] = {"", NULL, NULL, NULL};
		MessageBoxButton resultButtons[4] = {(MessageBoxButton)NULL, (MessageBoxButton)NULL, (MessageBoxButton)NULL, (MessageBoxButton)NULL};
		int indexCancel = -1;
		if ((buttonMask & MESSAGE_BUTTON_OK) && (buttonMask & MESSAGE_BUTTON_CANCEL))
		{
			// order is reversed because libKD prefers the colored button to be at place [1], at least on iOS
			// if this is going to be changed for a new platform, ifdef the button order for iOS
			buttons[1] = ok.c_str();
			buttons[0] = cancel.c_str();
			resultButtons[1] = MESSAGE_BUTTON_OK;
			resultButtons[0] = MESSAGE_BUTTON_CANCEL;
			indexCancel = 0;
		}
		else if ((buttonMask & MESSAGE_BUTTON_YES) && (buttonMask & MESSAGE_BUTTON_NO) && (buttonMask & MESSAGE_BUTTON_CANCEL))
		{
			buttons[1] = yes.c_str();
			buttons[0] = no.c_str();
			buttons[2] = cancel.c_str();
			resultButtons[1] = MESSAGE_BUTTON_YES;
			resultButtons[0] = MESSAGE_BUTTON_NO;
			resultButtons[2] = MESSAGE_BUTTON_CANCEL;
			indexCancel = 2;
		}
		else if (buttonMask & MESSAGE_BUTTON_OK)
		{
			buttons[0] = ok.c_str();
			resultButtons[0] = MESSAGE_BUTTON_OK;
			indexCancel = 0;
		}
		else if ((buttonMask & MESSAGE_BUTTON_YES) && (buttonMask & MESSAGE_BUTTON_NO))
		{
			buttons[1] = yes.c_str();
			buttons[0] = no.c_str();
			resultButtons[1] = MESSAGE_BUTTON_YES;
			resultButtons[0] = MESSAGE_BUTTON_NO;
			indexCancel = 1;
		}
		int index = kdShowMessage(title.c_str(), text.c_str(), buttons);
		if (index == -1)
		{
			index = indexCancel;
		}
		if (callback != NULL && index >= 0)
		{
			(*callback)(resultButtons[index]);
		}
	}

}
#endif
