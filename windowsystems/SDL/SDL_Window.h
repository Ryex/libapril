/// @file
/// @author  Ivan Vucica
/// @author  Kresimir Spes
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
/// Defines an SDL window.

#ifndef APRIL_SDL_WINDOW_H
#define APRIL_SDL_WINDOW_H

#ifdef _SDL_WINDOW

#include <SDL/SDL_keysym.h>
#include <hltypes/hstring.h>

#ifdef _OPENGLES
#include <SDL/SDL.h>
#include <SDL/SDL_gles.h>
#endif

#include "aprilExport.h"
#include "Window.h"

struct SDL_Surface;
union SDL_Event;

namespace april
{
	class SDL_Window : public Window
	{
	public:
		SDL_Window();
		~SDL_Window();
		bool create(int w, int h, bool fullscreen, chstr title, Window::Options options);
		bool destroy();
		
		void setTitle(chstr title);
		bool isCursorVisible();
		void setCursorVisible(bool visible);
		HL_DEFINE_IS(cursorInside, CursorInside);
		int getWidth();
		int getHeight();
		gvec2 getCursorPosition();
		void* getBackendId();
		void setResolution(int w, int h, bool fullscreen);

		bool updateOneFrame();
		void presentFrame();
		void checkEvents();
		
	protected:
		bool cursorInside;
		bool scrollHorizontal;
		SDL_Surface* screen;
		int videoBpp;
		unsigned int videoFlags;
#ifdef _OPENGLES
		SDL_GLES_Context* glesContext;
#endif

		float _calcTimeSinceLastFrame();
		void _handleSDLKeyEvent(Window::KeyEventType type, SDLKey keyCode, unsigned int unicode);
		void _handleSDLMouseEvent(SDL_Event &evt);		

	};

}

#endif
#endif
