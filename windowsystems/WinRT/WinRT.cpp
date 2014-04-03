/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _WINRT_WINDOW

#include <gtypes/Rectangle.h>
#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "IWinRT.h"
#include "Platform.h"
#include "WinRT.h"

namespace april
{
	void (*WinRT::Init)(const harray<hstr>&) = NULL;
	void (*WinRT::Destroy)() = NULL;
	harray<hstr> WinRT::Args;
	IWinRT^ WinRT::Interface = nullptr;
#ifndef _WINP8
	WinRT_XamlOverlay^ WinRT::XamlOverlay = nullptr;
#else
	int WinRT::getScreenRotation()
	{
		int rotation = 0;
		switch (DisplayProperties::NativeOrientation) // default is landscape
		{
		case DisplayOrientations::Portrait:
			rotation = 90;
			break;
		case DisplayOrientations::LandscapeFlipped:
			rotation = 180;
			break;
		case DisplayOrientations::PortraitFlipped:
			rotation = 270;
			break;
		}
		switch (DisplayProperties::CurrentOrientation)
		{
		case DisplayOrientations::PortraitFlipped:
			rotation = (rotation + 90) % 360;
			break;
		case DisplayOrientations::LandscapeFlipped:
			rotation = (rotation + 180) % 360;
			break;
		case DisplayOrientations::Portrait:
			rotation = (rotation + 270) % 360;
			break;
		}
		return rotation;
	}

	grect WinRT::rotateViewport(grect viewport)
	{
		static int w = 0;
		static int h = 0;
		if (w == 0 || h == 0)
		{
			gvec2 resolution = april::getSystemInfo().displayResolution;
			w = hround(resolution.x);
			h = hround(resolution.y);
			CHECK_SWAP(w, h);
		}
		int rotation = WinRT::getScreenRotation();
		if (rotation == 90)
		{
			hswap(viewport.x, viewport.y);
			hswap(viewport.w, viewport.h);
			viewport.x = w - (viewport.x + viewport.w);
		}
		else if (rotation == 180)
		{
			viewport.x = w - (viewport.x + viewport.w);
			viewport.y = h - (viewport.y + viewport.h);
		}
		else if (rotation == 270)
		{
			hswap(viewport.x, viewport.y);
			hswap(viewport.w, viewport.h);
			viewport.y = h - (viewport.y + viewport.h);
		}
		return viewport;
	}

#endif
	
}
#endif
