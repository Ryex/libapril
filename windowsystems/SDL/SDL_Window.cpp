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

#ifdef _SDL_WINDOW
#include <hltypes/hplatform.h>
#ifdef __APPLE__
#include <TargetConditionals.h>
#include <OpenGL/gl.h>
#elif _OPENGLES
#include <GLES/gl.h>
#else
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gl/GL.h>
#endif

#include <SDL/SDL.h>
#include <ctype.h> // tolower()
#include <SDL/SDL_syswm.h>

#include <hltypes/exception.h>
#include <hltypes/hlog.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "Keys.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "SDL_Window.h"
#ifdef _DIRECTX
#include "DirectX_RenderSystem.h"
#endif
#ifdef _OPENGL1
#include "OpenGL1_RenderSystem.h"
#endif
#ifdef _OPENGLES
#include "OpenGLES_RenderSystem.h"
#endif

extern int gAprilShouldInvokeQuitCallback;

namespace april
{
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
	void platform_cursorVisibilityUpdate();
	bool platform_CursorIsVisible();
#endif
	
	SDL_Window::SDL_Window() : Window()
	{
		this->name = APRIL_WS_SDL;
		// centered SDL window
#ifndef __APPLE__
		SDL_putenv("SDL_VIDEO_WINDOW_POS");
		SDL_putenv("SDL_VIDEO_CENTERED=1");
#endif
		this->cursorInside = true;
		this->scrollHorizontal = false;
		this->screen = NULL;
		this->created = false;
#ifdef _OPENGLES
		this->glesContext = NULL;
#endif
	}
	
	SDL_Window::~SDL_Window()
	{
		this->destroy();
	}
	
	bool SDL_Window::create(int w, int h, bool fullscreen, chstr title, Window::Options options)
	{
		if (!Window::create(w, h, fullscreen, title, options))
		{
			return false;
		}
		this->inputMode = MOUSE;
		this->cursorInside = true;
		// initialize only SDL video subsystem
		SDL_Init(SDL_INIT_VIDEO);
		// and immediately set window title
		SDL_WM_SetCaption(this->title.c_str(), this->title.c_str());
		this->videoFlags = (fullscreen ? SDL_FULLSCREEN : 0);
		if (this->options.resizable)
		{
			this->videoFlags |= SDL_RESIZABLE;
		}
		this->videoBpp = 0;
#ifdef _OPENGL
#ifdef _OPENGL1
		if (dynamic_cast<OpenGL1_RenderSystem*>(april::rendersys) != NULL)
		{
			// set up opengl attributes desired for the context
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
			this->videoFlags |= SDL_OPENGL;
			this->screen = SDL_SetVideoMode(w, h, this->videoBpp, this->videoFlags);
		}
#endif
#ifdef _OPENGLES
		if (dynamic_cast<OpenGLES_RenderSystem*>(april::rendersys) != NULL)
		{
			SDL_GLES_Init(SDL_GLES_VERSION_1_1);
			this->videoFlags = SDL_SWSURFACE;
			this->videoBpp = 16;
			this->screen = SDL_SetVideoMode(0, 0, this->videoBpp, this->videoFlags);
			this->glesContext = SDL_GLES_CreateContext();
			SDL_GLES_MakeCurrent(this->glesContext);
		}
#endif
		glClear(GL_COLOR_BUFFER_BIT);
#endif
#ifdef _DIRECTX
		if (dynamic_cast<DirectX_RenderSystem*>(april::rendersys) != NULL)
		{
			this->screen = SDL_SetVideoMode(w, h, this->videoBpp, this->videoFlags);
		}
#endif
		if (this->screen == NULL)
		{
#ifdef __APPLE__
#if !TARGET_OS_IPHONE
			// TODO elsewhere, add support for platform-specific msgbox code
			//NSRunAlertPanel(@"Could not open display", @"Game could not set the screen resolution. Perhaps resetting game configuration will help.", @"Ok", nil, nil);
#endif
#endif
			throw hl_exception("Requested display mode could not be provided!");
		}
#ifndef _WIN32
		april::rendersys->clear();
		april::rendersys->presentFrame();
#endif
		SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
		// not running yet
		this->running = false;
		// cursor is visible by default
		this->cursorVisible = true;
		// key repeat
		SDL_EnableKeyRepeat(100, 50);
		this->checkEvents();
		SDL_EnableUNICODE(1);
		this->created = true;
		return true;
	}
	
	bool SDL_Window::destroy()
	{
		if (!Window::destroy())
		{
			return false;
		}
#ifdef _OPENGLES
		SDL_GLES_DeleteContext(this->glesContext);
		SDL_GLES_Quit();
#endif
		SDL_Quit();
		return true;
	}
	
	void SDL_Window::setTitle(chstr value)
	{
		Window::setTitle(value);
		SDL_WM_SetCaption(this->title.c_str(), this->title.c_str());
	}
	
	bool SDL_Window::isCursorVisible()
	{
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
		return this->cursorVisible;
#else
		return (SDL_ShowCursor(SDL_QUERY) != 0);
#endif
	}
	
	void SDL_Window::setCursorVisible(bool value)
	{
		Window::setCursorVisible(value);
		// TODO - refactor
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
		// intentionally do nothing; let the platform specific code take over
		// if sdl-hidden mouse goes over the dock, then it will show again,
		// and won't be hidden again
#else
		// on other platforms, SDL does a good enough job
		SDL_ShowCursor(this->cursorVisible ? 1 : 0);
#endif
	}
	
	int SDL_Window::getWidth()
	{
		return SDL_GetVideoInfo()->current_w;
	}
	
	int SDL_Window::getHeight()
	{
		return SDL_GetVideoInfo()->current_h;
	}
	
	void* SDL_Window::getBackendId()
	{
#ifdef _WIN32
		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWMInfo(&wmInfo);
		return (void*)wmInfo.window;
#else
		// not implemented
		return NULL;
#endif
	}
	
	void SDL_Window::setResolution(int w, int h, bool fullscreen)
	{
		if (this->fullscreen == fullscreen && this->getWidth() == w && this->getHeight() == h)
		{
			return;
		}
		this->videoFlags = (fullscreen ? this->videoFlags | SDL_FULLSCREEN : this->videoFlags & ~SDL_FULLSCREEN);
		SDL_SetVideoMode(w, h, this->videoBpp, this->videoFlags);
	}
	
	bool SDL_Window::updateOneFrame()
	{
		// check if we should quit...
		if (gAprilShouldInvokeQuitCallback != 0)
		{
			SDL_Event sdlEvent;
			sdlEvent.type = SDL_QUIT;
			SDL_PushEvent(&sdlEvent);
			gAprilShouldInvokeQuitCallback = 0;
		}
		// first process sdl events
		this->checkEvents();
		return Window::updateOneFrame();
	}

	void SDL_Window::checkEvents()
	{
		SDL_Event sdlEvent;
		while (SDL_PollEvent(&sdlEvent))
		{
			switch (sdlEvent.type)
			{
			case SDL_VIDEORESIZE:
				SDL_SetVideoMode(sdlEvent.resize.w, sdlEvent.resize.h, this->videoBpp, this->videoFlags);
				this->_setRenderSystemResolution(sdlEvent.resize.w, sdlEvent.resize.h, this->fullscreen);
				break;
			case SDL_QUIT:
				if (this->handleQuitRequest(true))
				{
					this->running = false;
				}
				break;
			case SDL_KEYUP:
			case SDL_KEYDOWN:
#ifdef __APPLE__
				// on mac os, we need to handle command+q
				if (SDL_GetModState() & KMOD_META && (tolower(sdlEvent.key.keysym.unicode) == 'q' || sdlEvent.key.keysym.sym == SDLK_q))
				{
					if (this->handleQuitRequest(true))
					{
						this->running = false;
					}
				}
				else
#endif
				{
					this->_handleSDLKeyEvent((sdlEvent.type == SDL_KEYUP ? KEY_UP : KEY_DOWN),
						sdlEvent.key.keysym.sym, sdlEvent.key.keysym.unicode);
				}
				break;
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEMOTION:
				this->_handleSDLMouseEvent(sdlEvent);
				break;
			case SDL_ACTIVEEVENT:
				if (sdlEvent.active.state & SDL_APPINPUTFOCUS)
				{
					this->handleFocusChangeEvent(sdlEvent.active.gain != 0);
				}
				if (sdlEvent.active.state & SDL_APPMOUSEFOCUS)
				{
					if (sdlEvent.active.gain && this->isCursorVisible() && !this->cursorVisible)
					{
						SDL_ShowCursor(0);
					}
					this->cursorInside = (sdlEvent.active.gain != 0);
				}
				break;
			default:
				break;
			}
		}
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
		platform_cursorVisibilityUpdate();
#endif
	}
	
	void SDL_Window::presentFrame()
	{
#ifdef _OPENGL
#ifdef _OPENGL1
		if (april::rendersys->getName() == APRIL_RS_OPENGL1)
		{
			SDL_GL_SwapBuffers();
		}
#endif
#ifdef _OPENGLES
		harray<hstr> renderSystems;
		renderSystems += APRIL_RS_OPENGLES1;
		renderSystems += APRIL_RS_OPENGLES2;
		if (renderSystems.contains(april::rendersys->getName()))
		{
			SDL_GLES_SwapBuffers();
		}
#endif
#endif
	}
	
	void SDL_Window::_handleSDLKeyEvent(Window::KeyEventType type, SDLKey keysym, unsigned int unicode)
	{
		april::Key akeysym = AK_UNKNOWN;
	
		#define _s2a(sdlk, ak) case sdlk: akeysym = ak; break; 
		#define _s2a_u0(sdlk, ak) case sdlk: akeysym = ak; unicode = 0; break;
		#define s2a(sdlk, ak) _s2a(SDLK_ ## sdlk, AK_ ## ak)
		#define s2a_u0(sdlk, ak) _s2a_u0(SDLK_ ## sdlk, AK_ ## ak)
		#define sea(key) s2a(key, key)
		#define sea_u0(key) s2a_u0(key, key)
		
		switch (keysym)
		{
			// control character keys
			s2a(BACKSPACE, BACK);
#ifdef __APPLE__
			s2a_u0(DELETE, DELETE);
#else
		case SDLK_DELETE:
			akeysym = AK_DELETE; //sea(DELETE);
#endif
			sea(TAB)
			sea(RETURN);
			s2a(KP_ENTER, RETURN);
			
			// control keys above cursor keys
			s2a(PAGEUP, PRIOR);
			s2a(PAGEDOWN, NEXT);
			sea(HOME);
			sea(END);
			sea(INSERT);
			// delete already defined under control chars
			// this is because on mac sdl, delete == backspace
			// for some reason
			
			// cursor keys
			sea_u0(LEFT);
			sea_u0(RIGHT);
			sea_u0(UP);
			sea_u0(DOWN);
			
			// space
			sea(SPACE);
			
			// function keys
			sea_u0(F1);
			sea_u0(F2);
			sea_u0(F3);
			sea_u0(F4);
			sea_u0(F5);
			sea_u0(F6);
			sea_u0(F7);
			sea_u0(F8);
			sea_u0(F9);
			sea_u0(F10);
			sea_u0(F11);
			sea_u0(F12);
			s2a(ESCAPE, ESCAPE);
			
			// keypad keys
			s2a(KP0, NUMPAD0);
			s2a(KP1, NUMPAD1);
			s2a(KP2, NUMPAD2);
			s2a(KP3, NUMPAD3);
			s2a(KP4, NUMPAD4);
			s2a(KP5, NUMPAD5);
			s2a(KP6, NUMPAD6);
			s2a(KP7, NUMPAD7);
			s2a(KP8, NUMPAD8);
			s2a(KP9, NUMPAD9);
			
			s2a(LCTRL, LCONTROL);
			s2a(RCTRL, RCONTROL);
			s2a(LALT, LMENU);
			s2a(RALT, RMENU);
			s2a(LMETA, LCOMMAND);
			s2a(RMETA, RCOMMAND);
			s2a(LSHIFT, LSHIFT);
			s2a(RSHIFT, RSHIFT);
		default:
			break;
		}
		// number keys
		if (keysym >= '0' && keysym <= '9')
		{
			akeysym = (Key)keysym;
		}
		// letters
		if (keysym >= 'a' && keysym <= 'z') // sdl letter keys are small ASCII letters
		{
			akeysym = (Key)(keysym - 32); // april letter keys are capital ASCII letters
		}
		if (akeysym == AK_LCONTROL || akeysym == AK_RCONTROL)
		{
			if (type == KEY_DOWN)
			{
				this->scrollHorizontal = true;
			}
			else if (type == KEY_UP)
			{
				this->scrollHorizontal = false;
			}
		}
		Window::handleKeyEvent(type, akeysym, unicode);
	}
		
	void SDL_Window::_handleSDLMouseEvent(SDL_Event& sdlEvent)
	{
		this->cursorPosition.set(sdlEvent.button.x, sdlEvent.button.y);
		Window::MouseEventType mouseEvent;
		april::Key mouseButton = AK_NONE;

		switch (sdlEvent.type)
		{
		case SDL_MOUSEBUTTONUP:
			mouseEvent = MOUSE_UP;
			break;
		case SDL_MOUSEBUTTONDOWN:
			mouseEvent = MOUSE_DOWN;
			break;
		case SDL_MOUSEMOTION:
			mouseEvent = MOUSE_MOVE;
			break;
		default:
			break;
		}

		if (sdlEvent.type == SDL_MOUSEBUTTONUP || sdlEvent.type == SDL_MOUSEBUTTONDOWN)
		{
			switch (sdlEvent.button.button)
			{
			case SDL_BUTTON_LEFT:
				mouseButton = AK_LBUTTON;
				break;
			case SDL_BUTTON_RIGHT:
				mouseButton = AK_RBUTTON;
				break;
			case SDL_BUTTON_MIDDLE:
				mouseButton = AK_MBUTTON;
				break;
			case SDL_BUTTON_WHEELUP:
				if (sdlEvent.type == SDL_MOUSEBUTTONDOWN)
				{
					mouseButton = AK_WHEELUP;
					mouseEvent = MOUSE_SCROLL;
				}
				else return;
				break;
			case SDL_BUTTON_WHEELDOWN:
				if (sdlEvent.type == SDL_MOUSEBUTTONDOWN)
				{
					mouseButton = AK_WHEELDN;
					mouseEvent = MOUSE_SCROLL;
				}
				else return;
				break;
			default:
				mouseButton = AK_NONE;
				break;
			}
		}
		
		if (mouseButton == AK_WHEELUP || mouseButton == AK_WHEELDN)
		{
			gvec2 scroll;
			scroll.x = (!this->scrollHorizontal ? 0.0f : (mouseButton == AK_WHEELUP ? -1.0f : 1.0f));
			scroll.y = (this->scrollHorizontal ? 0.0f : (mouseButton == AK_WHEELUP ? -1.0f : 1.0f));
			this->handleMouseEvent(mouseEvent, scroll, mouseButton);
		}
		else
		{
			this->handleMouseEvent(mouseEvent, this->cursorPosition, mouseButton);
		}
	}
	
	float SDL_Window::_calcTimeSinceLastFrame()
	{
		static unsigned int x = SDL_GetTicks();
		float k = (SDL_GetTicks() - x) * 0.001f;
		x = SDL_GetTicks();
		if (k > 0.5f)
		{
			k = 0.05f; // prevent jumps. from eg, waiting on device reset or super low framerate
		}
		if (!this->focused)
		{
			k = 0.0f;
		}
		return k;
	}
		
}

#endif
