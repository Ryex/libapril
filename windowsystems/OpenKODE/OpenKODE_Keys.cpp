/// @file
/// @author  Kresimir Spes
/// @version 3.3
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _OPENKODE_WINDOW
#include <KD/kd.h>
#include <KD/kd_keyboard.h>
#include <hltypes/hlog.h>
#include <hltypes/hmap.h>
#include "april.h"
#include "OpenKODE_Keys.h"

namespace april
{
	static hmap<int, Key> gKeyMap;
	
	Key kd2april(int kdKeyCode)
	{
		if (gKeyMap.has_key(kdKeyCode))
		{
			return gKeyMap[kdKeyCode];
		}
#ifdef _DEBUG
		hlog::writef(logTag, "Unknown key code: %u", kdKeyCode);
#endif
		return april::AK_NONE;
	}
	
	void initOpenKODEKeyMap()
	{
		hmap<int, Key>& m = gKeyMap;
		m[KD_INPUT_KEYS_LBUTTON] = AK_LBUTTON;
		m[KD_INPUT_KEYS_RBUTTON] = AK_RBUTTON;
		m[KD_INPUT_KEYS_MBUTTON] = AK_MBUTTON;
		m[KD_INPUT_KEYS_ESCAPE] = AK_ESCAPE;
		m[KD_INPUT_KEYS_BACKSPACE] = AK_BACK;
		m[KD_INPUT_KEYS_TAB] = AK_TAB;
		m[KD_INPUT_KEYS_ENTER] = AK_RETURN;
		m[KD_INPUT_KEYS_SPACE] = AK_SPACE;
		m[KD_INPUT_KEYS_SHIFT] = AK_SHIFT;
		m[KD_INPUT_KEYS_CTRL] = AK_CONTROL;
		m[KD_INPUT_KEYS_ALT] = AK_MENU;
		m[KD_INPUT_KEYS_LWIN] = AK_LCOMMAND;
		m[KD_INPUT_KEYS_RWIN] = AK_RCOMMAND;
		m[KD_INPUT_KEYS_APPS] = AK_APPS;
		m[KD_INPUT_KEYS_PAUSE] = AK_PAUSE;
		m[KD_INPUT_KEYS_CAPSLOCK] = AK_CAPITAL;
		m[KD_INPUT_KEYS_NUMLOCK] = AK_NUMLOCK;
		m[KD_INPUT_KEYS_PGUP] = AK_PRIOR;
		m[KD_INPUT_KEYS_PGDN] = AK_NEXT;
		m[KD_INPUT_KEYS_HOME] = AK_HOME;
		m[KD_INPUT_KEYS_END] = AK_END;
		m[KD_INPUT_KEYS_INSERT] = AK_INSERT;
		m[KD_INPUT_KEYS_DELETE] = AK_DELETE;
		m[KD_INPUT_KEYS_LEFT] = AK_LEFT;
		m[KD_INPUT_KEYS_UP] = AK_UP;
		m[KD_INPUT_KEYS_RIGHT] = AK_RIGHT;
		m[KD_INPUT_KEYS_DOWN] = AK_DOWN;
		m[KD_INPUT_KEYS_0] = AK_0;
		m[KD_INPUT_KEYS_1] = AK_1;
		m[KD_INPUT_KEYS_2] = AK_2;
		m[KD_INPUT_KEYS_3] = AK_3;
		m[KD_INPUT_KEYS_4] = AK_4;
		m[KD_INPUT_KEYS_5] = AK_5;
		m[KD_INPUT_KEYS_6] = AK_6;
		m[KD_INPUT_KEYS_7] = AK_7;
		m[KD_INPUT_KEYS_8] = AK_8;
		m[KD_INPUT_KEYS_9] = AK_9;
		m[KD_INPUT_KEYS_A] = AK_A;
		m[KD_INPUT_KEYS_B] = AK_B;
		m[KD_INPUT_KEYS_C] = AK_C;
		m[KD_INPUT_KEYS_D] = AK_D;
		m[KD_INPUT_KEYS_E] = AK_E;
		m[KD_INPUT_KEYS_F] = AK_F;
		m[KD_INPUT_KEYS_G] = AK_G;
		m[KD_INPUT_KEYS_H] = AK_H;
		m[KD_INPUT_KEYS_I] = AK_I;
		m[KD_INPUT_KEYS_J] = AK_J;
		m[KD_INPUT_KEYS_K] = AK_K;
		m[KD_INPUT_KEYS_L] = AK_L;
		m[KD_INPUT_KEYS_M] = AK_M;
		m[KD_INPUT_KEYS_N] = AK_N;
		m[KD_INPUT_KEYS_O] = AK_O;
		m[KD_INPUT_KEYS_P] = AK_P;
		m[KD_INPUT_KEYS_Q] = AK_Q;
		m[KD_INPUT_KEYS_R] = AK_R;
		m[KD_INPUT_KEYS_S] = AK_S;
		m[KD_INPUT_KEYS_T] = AK_T;
		m[KD_INPUT_KEYS_U] = AK_U;
		m[KD_INPUT_KEYS_V] = AK_V;
		m[KD_INPUT_KEYS_W] = AK_W;
		m[KD_INPUT_KEYS_X] = AK_X;
		m[KD_INPUT_KEYS_Y] = AK_Y;
		m[KD_INPUT_KEYS_Z] = AK_Z;
		m[KD_INPUT_KEYS_NUMPAD0] = AK_NUMPAD0;
		m[KD_INPUT_KEYS_NUMPAD1] = AK_NUMPAD1;
		m[KD_INPUT_KEYS_NUMPAD2] = AK_NUMPAD2;
		m[KD_INPUT_KEYS_NUMPAD3] = AK_NUMPAD3;
		m[KD_INPUT_KEYS_NUMPAD4] = AK_NUMPAD4;
		m[KD_INPUT_KEYS_NUMPAD5] = AK_NUMPAD5;
		m[KD_INPUT_KEYS_NUMPAD6] = AK_NUMPAD6;
		m[KD_INPUT_KEYS_NUMPAD7] = AK_NUMPAD7;
		m[KD_INPUT_KEYS_NUMPAD8] = AK_NUMPAD8;
		m[KD_INPUT_KEYS_NUMPAD9] = AK_NUMPAD9;
		m[KD_INPUT_KEYS_MULTIPLY] = AK_MULTIPLY;
		m[KD_INPUT_KEYS_DIVIDE] = AK_DIVIDE;
		m[KD_INPUT_KEYS_ADD] = AK_ADD;
		m[KD_INPUT_KEYS_SUBTRACT] = AK_SUBTRACT;
		m[KD_INPUT_KEYS_DECIMAL] = AK_DECIMAL;
		m[KD_INPUT_KEYS_F1] = AK_F1;
		m[KD_INPUT_KEYS_F2] = AK_F2;
		m[KD_INPUT_KEYS_F3] = AK_F3;
		m[KD_INPUT_KEYS_F4] = AK_F4;
		m[KD_INPUT_KEYS_F5] = AK_F5;
		m[KD_INPUT_KEYS_F6] = AK_F6;
		m[KD_INPUT_KEYS_F7] = AK_F7;
		m[KD_INPUT_KEYS_F8] = AK_F8;
		m[KD_INPUT_KEYS_F9] = AK_F9;
		m[KD_INPUT_KEYS_F10] = AK_F10;
		m[KD_INPUT_KEYS_F11] = AK_F11;
		m[KD_INPUT_KEYS_F12] = AK_F12;
		m[KD_INPUT_KEYS_UNKNOWN] = AK_UNKNOWN;
		
//		m[KD_INPUT_KEYS_SCROLLLOCK] = AK_;
//		m[KD_INPUT_KEYS_GRAVE] = AK_;
//		m[KD_INPUT_KEYS_MINUS] = AK_;
//		m[KD_INPUT_KEYS_EQUALS] = AK_;
//		m[KD_INPUT_KEYS_BACKSLASH] = AK_;
//		m[KD_INPUT_KEYS_LBRACKET] = AK_;
//		m[KD_INPUT_KEYS_RBRACKET] = AK_;
//		m[KD_INPUT_KEYS_SEMICOLON] = AK_;
//		m[KD_INPUT_KEYS_APOSTROPHE] = AK_;
//		m[KD_INPUT_KEYS_COMMA] = AK_;
//		m[KD_INPUT_KEYS_PERIOD] = AK_;
//		m[KD_INPUT_KEYS_SLASH] = AK_;
	}
}

#endif
