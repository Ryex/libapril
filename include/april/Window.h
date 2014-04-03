/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic window.

#ifndef APRIL_WINDOW_H
#define APRIL_WINDOW_H

#include <gtypes/Vector2.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Keys.h"
#include "Timer.h"

namespace april
{
	class ControllerDelegate;
	class KeyboardDelegate;
	class MouseDelegate;
	class RenderSystem;
	class SystemDelegate;
	class TouchDelegate;
	class UpdateDelegate;

	class aprilExport Window
	{
	public:
		enum InputMode
		{
			MOUSE,
			TOUCH,
			CONTROLLER
		};
		
		enum MouseEventType
		{
			MOUSE_DOWN = 0,
			MOUSE_UP = 1,
			MOUSE_CANCEL = 2, // canceling a down event
			MOUSE_MOVE = 3,
			MOUSE_SCROLL = 4
		};

		enum KeyEventType
		{
			KEY_DOWN = 0,
			KEY_UP = 1
		};

		enum ControllerEventType
		{
			CONTROLLER_DOWN = 0,
			CONTROLLER_UP = 1
			// TODO - possibly add analog triggers
		};

		DEPRECATED_ATTRIBUTE static MouseEventType AMOUSEEVT_DOWN;
		DEPRECATED_ATTRIBUTE static MouseEventType AMOUSEEVT_UP;
		DEPRECATED_ATTRIBUTE static MouseEventType AMOUSEEVT_CANCEL;
		DEPRECATED_ATTRIBUTE static MouseEventType AMOUSEEVT_MOVE;
		DEPRECATED_ATTRIBUTE static MouseEventType AMOUSEEVT_SCROLL;
		DEPRECATED_ATTRIBUTE static KeyEventType AKEYEVT_DOWN;
		DEPRECATED_ATTRIBUTE static KeyEventType AKEYEVT_UP;
		DEPRECATED_ATTRIBUTE static ControllerEventType ACTRLEVT_DOWN;
		DEPRECATED_ATTRIBUTE static ControllerEventType ACTRLEVT_UP;
		
		struct MouseInputEvent
		{
			MouseEventType type;
			gvec2 position;
			Key keyCode;
		
			MouseInputEvent();
			MouseInputEvent(MouseEventType type, gvec2 position, Key keyCode);
		
		};

		struct KeyInputEvent
		{
			KeyEventType type;
			Key keyCode;
			unsigned int charCode;
			
			KeyInputEvent();
			KeyInputEvent(KeyEventType type, Key keyCode, unsigned int charCode);
		
		};

		struct TouchInputEvent
		{
			harray<gvec2> touches;
			
			TouchInputEvent();
			TouchInputEvent(harray<gvec2>& touches);
		
		};

		struct ControllerInputEvent
		{
			ControllerEventType type;
			Button buttonCode;
			
			ControllerInputEvent();
			ControllerInputEvent(ControllerEventType type, Button buttonCode);
		
		};

		struct aprilExport Options
		{
		public:
			bool resizable;
			bool fpsCounter;

			Options();
			~Options();

			hstr toString();

		};

		Window();
		virtual ~Window();
		virtual bool create(int w, int h, bool fullscreen, chstr title, Window::Options options);
		virtual bool destroy();
		virtual void unassign();

		// generic getters/setters
		HL_DEFINE_GET(hstr, name, Name);
		HL_DEFINE_GET(Options, options, Options);
		HL_DEFINE_IS(created, Created);
		HL_DEFINE_GET(hstr, title, Title);
		HL_DEFINE_IS(fullscreen, Fullscreen);
		void setFullscreen(bool value);
		HL_DEFINE_IS(focused, Focused);
		HL_DEFINE_IS(running, Running);
		HL_DEFINE_GETSET(int, fps, Fps);
		HL_DEFINE_GETSET(float, fpsResolution, FpsResolution);
		HL_DEFINE_GET(gvec2, cursorPosition, CursorPosition);
		HL_DEFINE_IS(virtualKeyboardVisible, VirtualKeyboardVisible);
		HL_DEFINE_GET(float, virtualKeyboardHeightRatio, VirtualKeyboardHeightRatio);
		HL_DEFINE_GET(InputMode, inputMode, InputMode);
		void setInputMode(InputMode value);
		HL_DEFINE_GET2(hmap, InputMode, InputMode, inputModeTranslations, InputModeTranslations);
		void setInputModeTranslations(hmap<InputMode, InputMode> value);
		HL_DEFINE_GET(hstr, cursorFilename, CursorFilename);
		gvec2 getSize();
		float getAspectRatio();
		
		HL_DEFINE_GETSET2(hmap, Key, Button, controllerEmulationKeys, ControllerEmulationKeys);

		// callbacks
		HL_DEFINE_GETSET(UpdateDelegate*, updateDelegate, UpdateDelegate);
		HL_DEFINE_GETSET(KeyboardDelegate*, keyboardDelegate, KeyboardDelegate);
		HL_DEFINE_GETSET(MouseDelegate*, mouseDelegate, MouseDelegate);
		HL_DEFINE_GETSET(TouchDelegate*, touchDelegate, TouchDelegate);
		HL_DEFINE_GETSET(ControllerDelegate*, controllerDelegate, ControllerDelegate);
		HL_DEFINE_GETSET(SystemDelegate*, systemDelegate, SystemDelegate);

		// virtual getters/setters
		virtual void setTitle(chstr value) { this->title = value; }
		virtual bool isCursorVisible() { return this->cursorVisible; }
		virtual void setCursorVisible(bool value) { this->cursorVisible = value; }
		virtual void setCursorFilename(chstr value) { this->cursorFilename = value; }
		virtual bool isCursorInside();

		virtual void setResolution(int w, int h);
		virtual void setResolution(int w, int h, bool fullscreen);
		
		// pure virtual getters/setters (window system dependent)
		virtual int getWidth() = 0;
		virtual int getHeight() = 0;
		DEPRECATED_ATTRIBUTE bool isTouchEnabled() { return (this->inputMode == TOUCH); }
		virtual void* getBackendId() = 0;

		// pure virtual methods (window system dependent)
		virtual void presentFrame() = 0;

		// misc virtuals
		virtual bool updateOneFrame();
		virtual void checkEvents();
		virtual void terminateMainLoop();
		virtual void beginKeyboardHandling() { }
		virtual void terminateKeyboardHandling() { }
		
		virtual bool isRotating() { return false; } // iOS/Android devices for example
		virtual hstr getParam(chstr param) { return ""; } // TODOaa - this should be refactored
		virtual void setParam(chstr param, chstr value) { } // TODOaa - this should be refactored
		
		// generic but overridable event handlers
		virtual void handleMouseEvent(MouseEventType type, gvec2 position, Key keyCode);
		virtual void handleKeyEvent(KeyEventType type, Key keyCode, unsigned int charCode);
		virtual void handleTouchEvent(const harray<gvec2>& touches);
		virtual void handleControllerEvent(ControllerEventType type, Button buttonCode);
		virtual bool handleQuitRequest(bool canCancel);
		virtual void handleFocusChangeEvent(bool focused);
		virtual void handleActivityChangeEvent(bool active);
		virtual void handleVirtualKeyboardChangeEvent(bool visible, float heightRatio);
		virtual void handleLowMemoryWarning();

		void handleKeyOnlyEvent(KeyEventType type, Key keyCode);
		void handleCharOnlyEvent(unsigned int charCode);

		virtual void queueKeyEvent(KeyEventType type, Key keyCode, unsigned int charCode);
		virtual void queueMouseEvent(MouseEventType type, gvec2 position, Key keyCode);
		virtual void queueTouchEvent(MouseEventType type, gvec2 position, int index);
		virtual void queueControllerEvent(ControllerEventType type, Button buttonCode);

		virtual void enterMainLoop();
		virtual bool performUpdate(float k);
		
		// TODOaa - refactor
		// the following functions should be temporary, it was added because I needed access to
		// iOS early initialization process. When april will be refactored this needs to be changed --kspes
		static void setLaunchCallback(void (*callback)(void*)) { msLaunchCallback = callback; }
		static void handleLaunchCallback(void* args);

	protected:
		bool created;
		hstr name;
		hstr title;
		bool fullscreen;
		Options options;
		bool focused;
		bool running;
		int fps;
		int fpsCount;
		float fpsTimer;
		float fpsResolution;
		gvec2 cursorPosition;
		bool cursorVisible;
		bool virtualKeyboardVisible;
		float virtualKeyboardHeightRatio;
		InputMode inputMode;
		hmap<InputMode, InputMode> inputModeTranslations;
		hstr cursorFilename;
		harray<hstr> cursorExtensions;
		bool multiTouchActive;
		harray<gvec2> touches;
		harray<KeyInputEvent> keyEvents;
		harray<MouseInputEvent> mouseEvents;
		harray<TouchInputEvent> touchEvents;
		harray<ControllerInputEvent> controllerEvents;
		Timer timer;
		hmap<Key, Button> controllerEmulationKeys;

		// TODOaa - refactor
		static void (*msLaunchCallback)(void*);

		UpdateDelegate* updateDelegate;
		KeyboardDelegate* keyboardDelegate;
		MouseDelegate* mouseDelegate;
		TouchDelegate* touchDelegate;
		ControllerDelegate* controllerDelegate;
		SystemDelegate* systemDelegate;

		virtual float _calcTimeSinceLastFrame();
		void _setRenderSystemResolution();
		virtual void _setRenderSystemResolution(int w, int h, bool fullscreen);
		virtual hstr _findCursorFile();

	};

	// global window shortcut variable
	aprilFnExport extern april::Window* window;

}
#endif
