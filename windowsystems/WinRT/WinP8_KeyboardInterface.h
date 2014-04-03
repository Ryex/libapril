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
/// Defines WinRT keyboard.

#if defined(_WINRT) && defined(_WINP8)
#ifndef APRIL_WINP8_KEYBOARD_INTERFACE_H
#define APRIL_WINP8_KEYBOARD_INTERFACE_H

#include <agile.h>

#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "WinRT_BaseApp.h"

using namespace Windows::Phone::UI::Core;
using namespace Windows::UI::Core;

namespace april
{
	ref class WinP8_KeyboardInterface sealed
	{
	public:
		WinP8_KeyboardInterface(CoreWindow^ window, WinRT_BaseApp^ app);

		void processEvents();
		void showKeyboard();
		void hideKeyboard();

	private:
		Platform::Agile<CoreWindow> window;
		KeyboardInputBuffer^ inputBuffer;
		CoreInputScope inputScope;

		~WinP8_KeyboardInterface();

	};
	
}

#endif
#endif
