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
/// Defines a WinRT XAML App.

#if defined(_WINRT_WINDOW) && !defined(_WINP8)
#ifndef APRIL_WINRT_XAML_APP_H
#define APRIL_WINRT_XAML_APP_H

#include <gtypes/Matrix4.h>
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>

#include "Color.h"
#include "IWinRT.h"
#include "pch.h"
#include "WinRT_BaseApp.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace Windows::UI::ViewManagement;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Markup;

namespace april
{
	class Texture;
	
	[Windows::Foundation::Metadata::WebHostHidden]
	ref class WinRT_XamlApp sealed : public Application, public IWinRT, public IComponentConnector
	{
	public:
		WinRT_XamlApp();
		virtual void Connect(int connectionId, Object^ target);
		
		virtual void unassignWindow();
		virtual void setCursorVisible(bool value);
		virtual void setCursorResourceId(unsigned int id);
		virtual bool canSuspendResume();
		virtual void updateViewState();
		virtual void checkEvents();
		virtual void showKeyboard();
		virtual void hideKeyboard();
		
	internal:
		void OnWindowActivationChanged( _In_ Object^ sender, _In_ WindowActivatedEventArgs^ args);
		void OnRender(_In_ Object^ sender, _In_ Object^ args);
		
	protected:
		void OnLaunched(_In_ LaunchActivatedEventArgs^ args) override;
		
	private:
		WinRT_BaseApp^ app;
		bool running;
		bool filled;
		bool snapped;
		Texture* logoTexture;
		Texture* splashTexture;
		bool hasStoredViewData;
		grect storedOrthoProjection;
		gmat4 storedProjectionMatrix;
		bool storedCursorVisible;
		unsigned int cursorResourceId;
		Color backgroundColor;
		bool launched;
		bool activated;
		Windows::Foundation::EventRegistrationToken eventToken;
		
		~WinRT_XamlApp();
		
		void _refreshCursor();
		void _tryRenderSplashTexture();
		april::Texture* _tryLoadTexture(chstr nodeName, chstr attributeName);
		void _tryLoadLogoTexture();
		void _tryLoadSplashTexture();
		void _tryLoadBackgroundColor();
		bool _findVisualElements(chstr nodeName, hstr& data, int& index);
		
	};

}
#endif
#endif
