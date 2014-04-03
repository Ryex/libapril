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
/// Defines main for standard C/C++.

#if !defined(_WIN32) || defined(_CONSOLE) && !defined(_WINRT) && !defined(_OPENKODE)
#ifndef APRIL_STANDARD_MAIN_H
#define APRIL_STANDARD_MAIN_H

#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "main_base.h"
#include "aprilExport.h"

int main(int argc, char** argv)
{
#ifdef __SINGLE_INSTANCE
	if (!april::__lockSingleInstanceMutex(hstr::from_unicode(__APRIL_SINGLE_INSTANCE_NAME), argv[0]))
	{
		return 0;
	}
#endif
	int result = april_main(april_init, april_destroy, argc, argv);
#ifdef __SINGLE_INSTANCE
	april::__unlockSingleInstanceMutex();
#endif
	return result;
}
#endif
#endif
