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
/// Defines a generic WinRT App.

#ifdef _WINRT_WINDOW
#ifndef APRIL_WINRT_BASE_APP_H
#define APRIL_WINRT_BASE_APP_H

#include <gtypes/Vector2.h>
#include <hltypes/harray.h>

#include "IWinRT.h"

using namespace Windows::ApplicationModel;
using namespace Windows::UI::Core;
using namespace Windows::UI::ViewManagement;

namespace april
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class WinRT_BaseApp sealed
	{
	public:
		WinRT_BaseApp();
		virtual ~WinRT_BaseApp();
		
		void assignEvents(CoreWindow^ window);
		
		HL_DEFINE_GET(unsigned int, startTime, StartTime);
		
		void OnSuspend(_In_ Object^ sender, _In_ SuspendingEventArgs^ args);
		void OnResume(_In_ Object^ sender, _In_ Object^ args);
		void OnWindowClosed(_In_ CoreWindow^ sender, _In_ CoreWindowEventArgs^ args);
		void OnWindowSizeChanged(_In_ CoreWindow^ sender, _In_ WindowSizeChangedEventArgs^ args);
		void OnVisibilityChanged(_In_ CoreWindow^ sender, _In_ VisibilityChangedEventArgs^ args);
		void OnOrientationChanged(_In_ Object^ args);
		void OnVirtualKeyboardShow(_In_ InputPane^ sender, _In_ InputPaneVisibilityEventArgs^ args);
		void OnVirtualKeyboardHide(_In_ InputPane^ sender, _In_ InputPaneVisibilityEventArgs^ args);
		
		void OnTouchDown(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args);
		void OnTouchUp(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args);
		void OnTouchMove(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args);
		void OnMouseScroll(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args);
		void OnKeyDown(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args);
		void OnKeyUp(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args);
		void OnCharacterReceived(_In_ CoreWindow^ sender, _In_ CharacterReceivedEventArgs^ args);
		
		void handleFocusChange(bool focused);
		void resetTouches();
		
	private:
		bool scrollHorizontal;
		harray<unsigned int> pointerIds;
		int mouseMoveMessagesCount;
		unsigned int startTime;
		april::Key currentButton;
		IWinRT^ iwinrt;
		
		gvec2 _transformPosition(float x, float y);
		
	};
}
#endif
#endif
