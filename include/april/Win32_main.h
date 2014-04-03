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
/// Defines main for Win32 "Windows" system.

#if defined(_WIN32) && !defined(_CONSOLE) && !defined(_WINRT)
#ifndef APRIL_WIN32_MAIN_H
#define APRIL_WIN32_MAIN_H

#include <stdio.h>
#include <shellapi.h>

#include <hltypes/harray.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "main_base.h"
#include "aprilExport.h"

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, wchar_t* wCmdLine, int nCmdShow)
{
	// extract arguments
	int argc = 0;
	wchar_t** wArgv = CommandLineToArgvW(wCmdLine, &argc);
#ifdef __SINGLE_INSTANCE
	if (!april::__lockSingleInstanceMutex(hstr::from_unicode(__APRIL_SINGLE_INSTANCE_NAME), hstr::from_unicode(wArgv[0])))
	{
		LocalFree(wArgv);
		return 0;
	}
#endif
	char** argv = new char*[argc];
	hstr arg;
	for_iter (i, 0, argc)
	{
		arg = hstr::from_unicode(wArgv[i]);
		argv[i] = new char[arg.size() + 1];
		memcpy(argv[i], arg.c_str(), sizeof(char) * (arg.size() + 1));
	}
	LocalFree(wArgv);
	// call the user specified main function
	int result = april_main(april_init, april_destroy, argc, argv);
	// free allocated memory for arguments
	for_iter (i, 0, argc)
	{
		delete [] argv[i];
	}
	delete [] argv;
#ifdef __SINGLE_INSTANCE
	april::__unlockSingleInstanceMutex();
#endif
	return result;
}
#endif
#endif
