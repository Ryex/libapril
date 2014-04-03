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
/// Defines main functions.

#ifndef APRIL_MAIN_BASE_H
#define APRIL_MAIN_BASE_H

#ifdef _ANDROID
#include <jni.h>
#include <string.h>
#endif

#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"

#ifdef __APRIL_SINGLE_INSTANCE_NAME
#define __SINGLE_INSTANCE
#endif

namespace april
{
	aprilFnExport bool __lockSingleInstanceMutex(hstr instanceName, chstr fallbackName);
	aprilFnExport void __unlockSingleInstanceMutex();
#ifdef _ANDROID
	aprilFnExport jint JNI_OnLoad(void (*anAprilInit)(const harray<hstr>&), void (*anAprilDestroy)(), JavaVM* vm, void* reserved);
#endif
}

#if !defined(_ANDROID) || defined(_OPENKODE)
aprilExport int april_main(void (*anAprilInit)(const harray<hstr>&), void (*anAprilDestroy)(), int argc, char** argv);
#endif

extern void april_init(const harray<hstr>& args);
extern void april_destroy();

#endif
