/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if defined(_WINRT) && !defined(_OPENKODE)
#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "IWinRT.h"
#include "main_base.h"
#include "RenderSystem.h"
#include "Window.h"
#include "WinP8_AppLauncher.h"
#include "WinRT.h"
#include "WinRT_XamlApp.h"

#ifndef _WINP8
using namespace Windows::UI::Xaml;
#else
using namespace Windows::ApplicationModel::Core;
#endif

int april_main(void (*anAprilInit)(const harray<hstr>&), void (*anAprilDestroy)(), int argc, char** argv)
{
	april::WinRT::Args.clear();
	if (argv != NULL && argv[0] != NULL)
	{
		for_iter (i, 0, argc)
		{
			april::WinRT::Args += argv[i];
		}
	}
	april::WinRT::Init = anAprilInit;
	april::WinRT::Destroy = anAprilDestroy;
#ifdef _WINRT_WINDOW
#ifndef _WINP8
	Application::Start(ref new ApplicationInitializationCallback(
		[](ApplicationInitializationCallbackParams^ p)
		{
			april::WinRT::Interface = ref new april::WinRT_XamlApp();
		}
	));
#else
	CoreApplication::Run(ref new april::WinP8_AppLauncher());
#endif
#endif
	return 0;
}
#endif
