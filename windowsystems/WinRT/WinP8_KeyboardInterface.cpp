/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if defined(_WINRT) && defined(_WINP8)
#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "WinRT_BaseApp.h"
#include "WinP8_KeyboardInterface.h"

using namespace Windows::Foundation;
using namespace Windows::Phone::UI::Core;
using namespace Windows::UI::Core;

namespace april
{
	WinP8_KeyboardInterface::WinP8_KeyboardInterface(CoreWindow^ window, WinRT_BaseApp^ app)
	{
		this->window = window;
		this->inputScope = CoreInputScope::Url;
		this->inputBuffer = ref new KeyboardInputBuffer();
		this->inputBuffer->Text = "";
	}

	WinP8_KeyboardInterface::~WinP8_KeyboardInterface()
	{
	}

	void WinP8_KeyboardInterface::processEvents()
	{
		if (this->inputBuffer->Text != "")
		{
			std::basic_string<unsigned int> text = _HL_PSTR_TO_HSTR(this->inputBuffer->Text).u_str();
			bool closed = false;
			for_itert (unsigned int, i, 0, text.size())
			{
				if (text[i] != '\n' && text[i] != '\r')
				{
					april::window->queueKeyEvent(april::Window::AKEYEVT_DOWN, AK_NONE, text[i]);
				}
				else
				{
					if (!closed)
					{
						april::window->terminateKeyboardHandling();
					}
					closed = true;
				}
			}
			this->inputBuffer->Text = "";
		}
	}

	void WinP8_KeyboardInterface::showKeyboard()
	{
		this->inputBuffer->InputScope = this->inputScope;
		this->window->KeyboardInputBuffer = this->inputBuffer;
		this->window->IsKeyboardInputEnabled = true;
	}

	void WinP8_KeyboardInterface::hideKeyboard()
	{
		this->window->IsKeyboardInputEnabled = false;
	}

}
#endif
