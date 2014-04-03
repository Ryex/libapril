/// @file
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if defined(_WIN32) && !defined(_OPENKODE) && !defined(_WINRT)
#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "main_base.h"
#include "RenderSystem.h"
#include "Window.h"

int gAprilShouldInvokeQuitCallback = 0;

int april_main(void (*anAprilInit)(const harray<hstr>&), void (*anAprilDestroy)(), int argc, char** argv)
{
	harray<hstr> args;
	if (argv != NULL && argv[0] != NULL)
	{
		for_iter (i, 0, argc)
		{
			args += argv[i];
		}
	}
	anAprilInit(args);
	april::window->enterMainLoop();
	anAprilDestroy();
	return 0;
}
#endif
