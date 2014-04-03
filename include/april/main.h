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
/// Defines main functions. Keep in mind following:
///
/// - You MUST include this header somewhere in your project (typically main.cpp).
/// - This header must only be included ONCE in the entire project.
/// - When using OpenKODE, include KD/kd.h before including this header.
/// 
/// Following preprocessors must be defined before including this header:
///
/// - Windows native: _WIN32
/// - Windows with console: _WIN32 _CONSOLE
/// - WinRT: _WINRT
/// - Android: _ANDROID
/// - Mac: __APPLE__
/// - iOS: __APPLE__
/// - Linux: _UNIX
/// - OpenKODE: _OPENKODE

#ifndef APRIL_MAIN_H
#define APRIL_MAIN_H

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#include <hltypes/hplatform.h>
#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "main_base.h"
#include "aprilExport.h"

#ifdef _OPENKODE
	#include "Android_main.h" // required because of JNI
	#include "OpenKODE_main.h"
#elif defined(_ANDROID)
	#include "Android_main.h"
#elif defined(_WINRT)
	#include "WinRT_main.h"
#elif defined(_WIN32) && !defined(_CONSOLE)
	#include "Win32_main.h"
#else
	#include "Standard_main.h"
#endif

#endif