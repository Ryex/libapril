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
/// Defines main for WinRT.

#if defined(_WIN32) && defined(_WINRT) && !defined(_OPENKODE)
#ifndef APRIL_WINRT_MAIN_H
#define APRIL_WINRT_MAIN_H

#include "aprilExport.h"

#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "main_base.h"
#include "aprilExport.h"

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^ args)
{
	// extract arguments
	int argc = 0;
	char** argv = new char*[argc];
	hstr arg;
	for_iter (i, 0, argc)
	{
		arg = hstr::from_unicode(args[i]->Data());
		argv[i] = new char[arg.size() + 1];
		memcpy(argv[i], arg.c_str(), sizeof(char) * (arg.size() + 1));
	}
	// call the user specified main function
	int result = april_main(april_init, april_destroy, argc, argv);
	// free allocated memory for arguments
	for_iter (i, 0, argc)
	{
		delete [] argv[i];
	}
	delete [] argv;
	return result;
}
#endif
#endif
