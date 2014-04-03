/// @file
/// @author  Kresimir Spes
/// @version 3.3
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _ANDROID

#include <hltypes/hlog.h>
#include <hltypes/hmap.h>

#include "april.h"
#include "AndroidJNI_Keys.h"

namespace april
{
	static hmap<int, Key> gKeyMap;
	
	Key android2april(int androidKeyCode)
	{
		if (gKeyMap.has_key(androidKeyCode))
		{
			return gKeyMap[androidKeyCode];
		}
#ifdef _DEBUG
		hlog::writef(logTag, "Unknown key code: %u", androidKeyCode);
#endif
		return april::AK_NONE;
	}
	
	void initAndroidKeyMap()
	{
		hmap<int, Key>& m = gKeyMap;

		// codes obtained from http://developer.android.com/reference/android/view/KeyEvent.html#KEYCODE_0
		m[-1] = AK_UNKNOWN;
		m[0] = AK_NONE;
		
		// LBUTTON and RBUTTON are necessary
		m[1] = AK_LBUTTON;
		m[2] = AK_RBUTTON;
		// these mostly don't exist on Android
		//m[0] = AK_CANCEL;
		//m[0] = AK_MBUTTON;
		//m[168] = AK_WHEELUP; // ZOOM_IN
		//m[169] = AK_WHEELDN; // ZOOM_OUT
		//m[0] = AK_DOUBLETAP;
		
		// most common keys; some not supported
		m[67] = AK_BACK;
		m[61] = AK_TAB;
		m[28] = AK_CLEAR;
		m[66] = AK_RETURN;
		//m[0] = AK_SHIFT;
		//m[0] = AK_CONTROL;
		m[82] = AK_MENU;
		//m[0] = AK_PAUSE;
		m[115] = AK_CAPITAL;
		
		// various keys for asian keyboards not supported
		//m[0] = AK_KANA;
		//m[0] = AK_HANGEUL;
		//m[0] = AK_HANGUL;
		//m[0] = AK_JUNJA;
		//m[0] = AK_FINAL;
		//m[0] = AK_HANJA;
		//m[0] = AK_KANJI;
		m[4] = AK_ESCAPE; // using Android's back button for this
		//m[0] = AK_CONVERT;
		//m[0] = AK_NONCONVERT;
		//m[0] = AK_ACCEPT;
		//m[0] = AK_MODECHANGE;
		
		// Mac CMD keys; they aren't used so they will act like CTRL
		//m[113] = AK_LCOMMAND;
		//m[114] = AK_RCOMMAND;
		
		// space
		m[62] = AK_SPACE;
		
		// don't exist on Android
		//m[0] = AK_PRIOR;
		//m[0] = AK_NEXT;
		//m[0] = AK_END;
		//m[0] = AK_HOME;
		
		// don't exist on Android
		//m[0] = AK_LEFT;
		//m[0] = AK_UP;
		//m[0] = AK_RIGHT;
		//m[0] = AK_DOWN;
		
		// don't exist on Android
		//m[0] = AK_SELECT;
		//m[0] = AK_PRINT;
		//m[0] = AK_EXECUTE;
		//m[0] = AK_SNAPSHOT;
		
		// some more keys
		m[124] = AK_INSERT;
		m[112] = AK_DELETE;
		//m[0] = AK_HELP;
		
		// '0'-'9'
		m[7] = AK_0;
		m[8] = AK_1;
		m[9] = AK_2;
		m[10] = AK_3;
		m[11] = AK_4;
		m[12] = AK_5;
		m[13] = AK_6;
		m[14] = AK_7;
		m[15] = AK_8;
		m[16] = AK_9;
		
		// 'A'-'Z'
		m[29] = AK_A;
		m[30] = AK_B;
		m[31] = AK_C;
		m[32] = AK_D;
		m[33] = AK_E;
		m[34] = AK_F;
		m[35] = AK_G;
		m[36] = AK_H;
		m[37] = AK_I;
		m[38] = AK_J;
		m[39] = AK_K;
		m[40] = AK_L;
		m[41] = AK_M;
		m[42] = AK_N;
		m[43] = AK_O;
		m[44] = AK_P;
		m[45] = AK_Q;
		m[46] = AK_R;
		m[47] = AK_S;
		m[48] = AK_T;
		m[49] = AK_U;
		m[50] = AK_V;
		m[51] = AK_W;
		m[52] = AK_X;
		m[53] = AK_Y;
		m[54] = AK_Z;
		
		// don't exist on Android
		//m[0] = AK_LWIN 0;
		//m[0] = AK_RWIN 0;
		//m[0] = AK_APPS 0;
		//m[0] = AK_SLEEP 0;
		
		// numpad
		m[144] = AK_NUMPAD0;
		m[145] = AK_NUMPAD1;
		m[146] = AK_NUMPAD2;
		m[147] = AK_NUMPAD3;
		m[148] = AK_NUMPAD4;
		m[149] = AK_NUMPAD5;
		m[150] = AK_NUMPAD6;
		m[151] = AK_NUMPAD7;
		m[152] = AK_NUMPAD8;
		m[153] = AK_NUMPAD9;
		m[155] = AK_MULTIPLY;
		m[157] = AK_ADD;
		m[159] = AK_SEPARATOR;
		m[156] = AK_SUBTRACT;
		m[158] = AK_DECIMAL;
		m[154] = AK_DIVIDE;
		
		// F-keys
		m[131] = AK_F1;
		m[132] = AK_F2;
		m[133] = AK_F3;
		m[134] = AK_F4;
		m[135] = AK_F5;
		m[136] = AK_F6;
		m[137] = AK_F7;
		m[138] = AK_F8;
		m[139] = AK_F9;
		m[140] = AK_F10;
		m[141] = AK_F11;
		m[142] = AK_F12;
		// don't exist on Android
		m[0] = AK_F13;
		m[0] = AK_F14;
		m[0] = AK_F15;
		m[0] = AK_F16;
		m[0] = AK_F17;
		m[0] = AK_F18;
		m[0] = AK_F19;
		m[0] = AK_F20;
		m[0] = AK_F21;
		m[0] = AK_F22;
		m[0] = AK_F23;
		m[0] = AK_F24;
		
		// don't exist on Android
		m[143] = AK_NUMLOCK;
		m[116] = AK_SCROLL;
		
		// specific left-and-right-shift keys
		m[59] = AK_LSHIFT;
		m[60] = AK_RSHIFT;
		m[113] = AK_LCONTROL;
		m[114] = AK_RCONTROL;
		m[57] = AK_LMENU;
		m[58] = AK_RMENU;
		
		// browser control keys
		//m[4] = AK_BROWSER_BACK;
		//m[125] = AK_BROWSER_FORWARD;
		//m[0] = AK_BROWSER_REFRESH;
		//m[0] = AK_BROWSER_STOP;
		//m[84] = AK_BROWSER_SEARCH;
		//m[0] = AK_BROWSER_FAVORITES;
		//m[0] = AK_BROWSER_HOME;
		
		// volume keys
		m[164] = AK_VOLUME_MUTE;
		m[25] = AK_VOLUME_DOWN;
		m[24] = AK_VOLUME_UP;
		
		// don't exist on Android
		//m[0] = AK_MEDIA_NEXT_TRACK;
		//m[0] = AK_MEDIA_PREV_TRACK;
		//m[0] = AK_MEDIA_STOP;
		//m[0] = AK_MEDIA_PLAY_PAUSE;
		
		// don't exist on Android
		//m[0] = AK_LAUNCH_MAIL;
		//m[0] = AK_LAUNCH_MEDIA_SELECT;
		//m[0] = AK_LAUNCH_APP1;
		//m[0] = AK_LAUNCH_APP2;
		
		// don't exist on Android
		//m[0] = AK_OEM_2;
		//m[0] = AK_OEM_3;
		//m[0] = AK_OEM_4;
		//m[0] = AK_OEM_5;
		//m[0] = AK_OEM_6;
		//m[0] = AK_OEM_7;
		//m[0] = AK_OEM_8;
		//m[0] = AK_OEM_102;
		
		// don't exist on Android
		//m[0] = AK_PACKET;
		//m[0] = AK_ATTN;
		//m[0] = AK_CRSEL;
		//m[0] = AK_EXSEL;
		//m[0] = AK_EREOF;
		
		// don't exist on Android
		//m[0] = AK_PLAY;
		//m[0] = AK_ZOOM;
		
		// don't exist on Android
		//m[0] = AK_NONAME;
		//m[0] = AK_PA1;
		//m[0] = AK_OEM_CLEAR;
	}
}
#endif
