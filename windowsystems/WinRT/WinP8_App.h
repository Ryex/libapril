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
/// Defines a WinRT View.

#if defined(_WINRT_WINDOW) && defined(_WINP8)
#ifndef APRIL_WINP8_APP_H
#define APRIL_WINP8_APP_H

#include <agile.h>

#include <hltypes/harray.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "IWinRT.h"
#include "Window.h"
#include "WinP8_KeyboardInterface.h"
#include "WinRT_BaseApp.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Phone::UI::Input;

namespace april
{
	ref class WinP8_App : public IWinRT, public IFrameworkView
	{
	public:
		virtual void Initialize(_In_ CoreApplicationView^ applicationView);
		virtual void Uninitialize();
		virtual void SetWindow(_In_ CoreWindow^ window);
		virtual void Load(_In_ Platform::String^ entryPoint);
		virtual void Run();
		
		CoreWindow^ getCoreWindow() { return this->window.Get(); }
		
		void OnActivated(_In_ CoreApplicationView^ applicationView, _In_ IActivatedEventArgs^ args);
		void OnOrientationChanged(_In_ Object^ sender);
		void OnLogicalDpiChanged(_In_ Object^ sender);
		void OnBackButtonPressed(Object^ sender, BackPressedEventArgs^ args);
		
		virtual void unassignWindow();
		virtual void setCursorVisible(bool value);
		virtual void setCursorResourceId(unsigned int id);
		virtual bool canSuspendResume();
		virtual void updateViewState();
		virtual void checkEvents();
		virtual void showKeyboard();
		virtual void hideKeyboard();

	private: // has to be private
		Platform::Agile<CoreWindow> window;
		WinRT_BaseApp^ app;
		WinP8_KeyboardInterface^ keyboardInterface;
		bool lowMemoryReported;
		
	};
	
}

#endif
#endif
