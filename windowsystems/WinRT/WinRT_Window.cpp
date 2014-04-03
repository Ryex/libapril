/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _WINRT_WINDOW
#include <hltypes/hfile.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hlog.h>
#include <hltypes/hrdir.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "RenderSystem.h"
#include "SystemDelegate.h"
#include "Timer.h"
#include "WinRT.h"
#include "WinRT_Window.h"

using namespace Windows::UI::ViewManagement;

namespace april
{
	WinRT_Window::WinRT_Window() : Window()
	{
		this->name = APRIL_WS_WINRT;
		this->width = 0;
		this->height = 0;
		this->delaySplash = 0.0f;
		this->allowFilledView = false;
		this->useCustomSnappedView = false;
		this->backButtonSystemHandling = false;
		this->cursorExtensions += ".ani";
		this->cursorExtensions += ".cur";
	}

	WinRT_Window::~WinRT_Window()
	{
		this->destroy();
	}

	bool WinRT_Window::create(int w, int h, bool fullscreen, chstr title, Window::Options options)
	{
#ifndef _WINP8
		if (ApplicationView::Value == ApplicationViewState::Filled)
		{
			w -= WINRT_SNAPPED_VIEW_UNUSED;
		}
#endif
		if (!Window::create(w, h, fullscreen, title, options))
		{
			return false;
		}
		this->width = w;
		this->height = h;
		this->delaySplash = 0.0f;
		this->allowFilledView = false;
		this->useCustomSnappedView = false;
		this->backButtonSystemHandling = false;
		this->cursorMappings.clear();
		this->inputMode = TOUCH;
		this->setCursorVisible(true);
		return true;
	}
	
	void WinRT_Window::unassign()
	{
		WinRT::Interface->unassignWindow();
		Window::unassign();
	}

	hstr WinRT_Window::getParam(chstr param)
	{
#ifndef _WINP8
		if (param == WINRT_ALLOW_FILLED_VIEW)
		{
			return hstr(this->allowFilledView ? "1" : "0");
		}
		if (param == WINRT_USE_CUSTOM_SNAPPED_VIEW)
		{
			return hstr(this->useCustomSnappedView ? "1" : "0");
		}
		if (param == WINRT_VIEW_STATE)
		{
			if (ApplicationView::Value == ApplicationViewState::Snapped)
			{
				return WINRT_VIEW_STATE_SNAPPED;
			}
			if (ApplicationView::Value == ApplicationViewState::Filled)
			{
				return WINRT_VIEW_STATE_FILLED;
			}
			return WINRT_VIEW_STATE_FULLSCREEN;
		}
		if (param == WINRT_CURSOR_MAPPINGS)
		{
			harray<hstr> mappings;
			foreach_m (unsigned int, it, this->cursorMappings)
			{
				mappings += hsprintf("%u %s", it->second, it->first.c_str());
			}
			return mappings.join('\n');
		}
		if (param == WINRT_DELAY_SPLASH)
		{
			return hstr(this->delaySplash);
		}
#else
		if (param == WINP8_BACK_BUTTON_SYSTEM_HANDLING)
		{
			return hstr(this->backButtonSystemHandling ? "1" : "0");
		}
#endif
		return "";
	}

	void WinRT_Window::setParam(chstr param, chstr value)
	{
#ifndef _WINP8
		if (param == WINRT_ALLOW_FILLED_VIEW)
		{
			this->allowFilledView = (value != "0");
		}
		if (param == WINRT_USE_CUSTOM_SNAPPED_VIEW)
		{
			this->useCustomSnappedView = (value != "0");
		}
		if (param == WINRT_VIEW_STATE)
		{
			if (ApplicationView::Value == ApplicationViewState::Snapped)
			{
				if (value != WINRT_VIEW_STATE_SNAPPED)
				{
					if (!ApplicationView::TryUnsnap())
					{
#ifdef _DEBUG
						hlog::warn(april::logTag, "Unsnapping failed!");
#endif
					}
				}
				else
				{
					hlog::warn(april::logTag, "Ignoring unsnap, new view state is already 'snapped'!");
				}
			}
			else
			{
				hlog::warn(april::logTag, "Application not in snapped view, cannot change view state!");
			}
		}
		if (param == WINRT_CURSOR_MAPPINGS)
		{
			this->cursorMappings.clear();
			harray<hstr> lines = value.split('\n', -1, true);
			harray<hstr> data;
			foreach (hstr, it, lines)
			{
				data = (*it).split(' ', 1);
				if (data.size() == 2)
				{
					this->cursorMappings[data[1]] = (unsigned int)data[0];
				}
			}
		}
		if (param == WINRT_DELAY_SPLASH)
		{
			this->delaySplash = (float)value;
		}
#else
		if (param == WINP8_BACK_BUTTON_SYSTEM_HANDLING)
		{
			this->backButtonSystemHandling = (value != "0");
		}
#endif
	}

	void WinRT_Window::setTitle(chstr title)
	{
		hlog::warn(april::logTag, "Window::setTitle() does nothing on WinRT.");
	}
	
	void WinRT_Window::setCursorVisible(bool value)
	{
		Window::setCursorVisible(value);
		WinRT::Interface->setCursorVisible(value);
	}

	void WinRT_Window::setCursorFilename(chstr value)
	{
		Window::setCursorFilename(value);
		WinRT::Interface->setCursorResourceId((unsigned int)this->_findCursorFile());
	}

	void* WinRT_Window::getBackendId()
	{
		// TODO ?
		return 0;
	}

	void WinRT_Window::setResolution(int w, int h, bool fullscreen)
	{
		hlog::error(april::logTag, "Cannot change resolution on window system: " + this->name);
	}

	void WinRT_Window::changeSize(int w, int h)
	{
		if (this->width != w || this->height != h)
		{
			this->width = w;
			this->height = h;
			this->_setRenderSystemResolution(w, h, this->fullscreen);
		}
	}
	
	void WinRT_Window::presentFrame()
	{
	}

	void WinRT_Window::checkEvents()
	{
		Window::checkEvents();
		WinRT::Interface->checkEvents();
	}
	
	void WinRT_Window::beginKeyboardHandling()
	{
		WinRT::Interface->showKeyboard();
	}

	void WinRT_Window::terminateKeyboardHandling()
	{
		WinRT::Interface->hideKeyboard();
	}

	hstr WinRT_Window::_findCursorFile()
	{
		if (this->cursorFilename == "")
		{
			return "0";
		}
		foreach (hstr, it, this->cursorExtensions)
		{
			if (this->cursorMappings.has_key(this->cursorFilename))
			{
				return hstr(this->cursorMappings[this->cursorFilename]);
			}
		}
		return "0";
	}
	
}
#endif
