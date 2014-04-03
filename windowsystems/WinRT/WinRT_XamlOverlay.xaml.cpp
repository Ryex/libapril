/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if defined(_WINRT_WINDOW) && !defined(_WINP8)
#include "pch.h"

#include "WinRT_XamlOverlay.xaml.h"

using namespace Windows::UI::Xaml;

namespace april
{
	WinRT_XamlOverlay::WinRT_XamlOverlay()
	{
		this->InitializeComponent();
	}

	void WinRT_XamlOverlay::showKeyboard()
	{
		this->keyboardTextbox->IsEnabled = true;
		this->keyboardTextbox->Focus(FocusState::Programmatic);
	}
	
	void WinRT_XamlOverlay::hideKeyboard()
	{
		this->keyboardDisable->Focus(FocusState::Programmatic);
	}
	
}
#endif
