/// @file
/// @author  Kresimir Spes
/// @version 3.3
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
#include <hltypes/hmap.h>
#include "Mac_Keys.h"
#import <Foundation/Foundation.h>


enum CarbonKeyCode
{
	kVK_ANSI_A                    = 0x00,
	kVK_ANSI_S                    = 0x01,
	kVK_ANSI_D                    = 0x02,
	kVK_ANSI_F                    = 0x03,
	kVK_ANSI_H                    = 0x04,
	kVK_ANSI_G                    = 0x05,
	kVK_ANSI_Z                    = 0x06,
	kVK_ANSI_X                    = 0x07,
	kVK_ANSI_C                    = 0x08,
	kVK_ANSI_V                    = 0x09,
	kVK_ANSI_B                    = 0x0B,
	kVK_ANSI_Q                    = 0x0C,
	kVK_ANSI_W                    = 0x0D,
	kVK_ANSI_E                    = 0x0E,
	kVK_ANSI_R                    = 0x0F,
	kVK_ANSI_Y                    = 0x10,
	kVK_ANSI_T                    = 0x11,
	kVK_ANSI_1                    = 0x12,
	kVK_ANSI_2                    = 0x13,
	kVK_ANSI_3                    = 0x14,
	kVK_ANSI_4                    = 0x15,
	kVK_ANSI_6                    = 0x16,
	kVK_ANSI_5                    = 0x17,
	kVK_ANSI_Equal                = 0x18,
	kVK_ANSI_9                    = 0x19,
	kVK_ANSI_7                    = 0x1A,
	kVK_ANSI_Minus                = 0x1B,
	kVK_ANSI_8                    = 0x1C,
	kVK_ANSI_0                    = 0x1D,
	kVK_ANSI_RightBracket         = 0x1E,
	kVK_ANSI_O                    = 0x1F,
	kVK_ANSI_U                    = 0x20,
	kVK_ANSI_LeftBracket          = 0x21,
	kVK_ANSI_I                    = 0x22,
	kVK_ANSI_P                    = 0x23,
	kVK_ANSI_L                    = 0x25,
	kVK_ANSI_J                    = 0x26,
	kVK_ANSI_Quote                = 0x27,
	kVK_ANSI_K                    = 0x28,
	kVK_ANSI_Semicolon            = 0x29,
	kVK_ANSI_Backslash            = 0x2A,
	kVK_ANSI_Comma                = 0x2B,
	kVK_ANSI_Slash                = 0x2C,
	kVK_ANSI_N                    = 0x2D,
	kVK_ANSI_M                    = 0x2E,
	kVK_ANSI_Period               = 0x2F,
	kVK_ANSI_Grave                = 0x32,
	kVK_ANSI_KeypadDecimal        = 0x41,
	kVK_ANSI_KeypadMultiply       = 0x43,
	kVK_ANSI_KeypadPlus           = 0x45,
	kVK_ANSI_KeypadClear          = 0x47,
	kVK_ANSI_KeypadDivide         = 0x4B,
	kVK_ANSI_KeypadEnter          = 0x4C,
	kVK_ANSI_KeypadMinus          = 0x4E,
	kVK_ANSI_KeypadEquals         = 0x51,
	kVK_ANSI_Keypad0              = 0x52,
	kVK_ANSI_Keypad1              = 0x53,
	kVK_ANSI_Keypad2              = 0x54,
	kVK_ANSI_Keypad3              = 0x55,
	kVK_ANSI_Keypad4              = 0x56,
	kVK_ANSI_Keypad5              = 0x57,
	kVK_ANSI_Keypad6              = 0x58,
	kVK_ANSI_Keypad7              = 0x59,
	kVK_ANSI_Keypad8              = 0x5B,
	kVK_ANSI_Keypad9              = 0x5C,

/* keycodes for keys that are independent of keyboard layout*/
	kVK_Return                    = 0x24,
	kVK_Tab                       = 0x30,
	kVK_Space                     = 0x31,
	kVK_Delete                    = 0x33,
	kVK_Escape                    = 0x35,
	kVK_Command                   = 0x37,
	kVK_Shift                     = 0x38,
	kVK_CapsLock                  = 0x39,
	kVK_Option                    = 0x3A,
	kVK_Control                   = 0x3B,
	kVK_RightShift                = 0x3C,
	kVK_RightOption               = 0x3D,
	kVK_RightControl              = 0x3E,
	kVK_Function                  = 0x3F,
	kVK_F17                       = 0x40,
	kVK_VolumeUp                  = 0x48,
	kVK_VolumeDown                = 0x49,
	kVK_Mute                      = 0x4A,
	kVK_F18                       = 0x4F,
	kVK_F19                       = 0x50,
	kVK_F20                       = 0x5A,
	kVK_F5                        = 0x60,
	kVK_F6                        = 0x61,
	kVK_F7                        = 0x62,
	kVK_F3                        = 0x63,
	kVK_F8                        = 0x64,
	kVK_F9                        = 0x65,
	kVK_F11                       = 0x67,
	kVK_F13                       = 0x69,
	kVK_F16                       = 0x6A,
	kVK_F14                       = 0x6B,
	kVK_F10                       = 0x6D,
	kVK_F12                       = 0x6F,
	kVK_F15                       = 0x71,
	kVK_Help                      = 0x72,
	kVK_Home                      = 0x73,
	kVK_PageUp                    = 0x74,
	kVK_ForwardDelete             = 0x75,
	kVK_F4                        = 0x76,
	kVK_End                       = 0x77,
	kVK_F2                        = 0x78,
	kVK_PageDown                  = 0x79,
	kVK_F1                        = 0x7A,
	kVK_LeftArrow                 = 0x7B,
	kVK_RightArrow                = 0x7C,
	kVK_DownArrow                 = 0x7D,
	kVK_UpArrow                   = 0x7E,

	kVK_ISO_Section               = 0x0A,

	kVK_JIS_Yen                   = 0x5D,
	kVK_JIS_Underscore            = 0x5E,
	kVK_JIS_KeypadComma           = 0x5F,
	kVK_JIS_Eisu                  = 0x66,
	kVK_JIS_Kana                  = 0x68
};

namespace april
{
	static hmap<unsigned int, Key> gMacKeyMap;
	
	Key getAprilMacKeyCode(unsigned int macKeyCode)
	{
		if (gMacKeyMap.has_key(macKeyCode)) return gMacKeyMap[macKeyCode];

#ifdef _DEBUG
		NSLog(@"Unknown key code: %u", macKeyCode);
#endif
		return april::AK_NONE;
	}
	
	void initMacKeyMap()
	{
		hmap<unsigned int, Key>& m = gMacKeyMap;
		

		m[kVK_ANSI_A] = AK_A;
		m[kVK_ANSI_S] = AK_S;
		m[kVK_ANSI_D] = AK_D;
		m[kVK_ANSI_F] = AK_F;
		m[kVK_ANSI_H] = AK_H;
		m[kVK_ANSI_G] = AK_G;
		m[kVK_ANSI_Z] = AK_Z;
		m[kVK_ANSI_X] = AK_X;
		m[kVK_ANSI_C] = AK_C;
		m[kVK_ANSI_V] = AK_V;
		m[kVK_ANSI_B] = AK_B;
		m[kVK_ANSI_Q] = AK_Q;
		m[kVK_ANSI_W] = AK_W;
		m[kVK_ANSI_E] = AK_E;
		m[kVK_ANSI_R] = AK_R;
		m[kVK_ANSI_Y] = AK_Y;
		m[kVK_ANSI_T] = AK_T;
		m[kVK_ANSI_1] = AK_1;
		m[kVK_ANSI_2] = AK_2;
		m[kVK_ANSI_3] = AK_3;
		m[kVK_ANSI_4] = AK_4;
		m[kVK_ANSI_6] = AK_5;
		m[kVK_ANSI_5] = AK_6;
		m[kVK_ANSI_9] = AK_9;
		m[kVK_ANSI_7] = AK_7;
		m[kVK_ANSI_8] = AK_8;
		m[kVK_ANSI_0] = AK_0;
		m[kVK_ANSI_O] = AK_O;
		m[kVK_ANSI_U] = AK_U;
		m[kVK_ANSI_I] = AK_I;
		m[kVK_ANSI_P] = AK_P;
		m[kVK_ANSI_L] = AK_L;
		m[kVK_ANSI_J] = AK_J;
		m[kVK_ANSI_K] = AK_K;
		m[kVK_ANSI_N] = AK_N;
		m[kVK_ANSI_M] = AK_M;
		m[kVK_ANSI_Keypad0] = AK_NUMPAD0;
		m[kVK_ANSI_Keypad1] = AK_NUMPAD1;
		m[kVK_ANSI_Keypad2] = AK_NUMPAD2;
		m[kVK_ANSI_Keypad3] = AK_NUMPAD3;
		m[kVK_ANSI_Keypad4] = AK_NUMPAD4;
		m[kVK_ANSI_Keypad5] = AK_NUMPAD5;
		m[kVK_ANSI_Keypad6] = AK_NUMPAD6;
		m[kVK_ANSI_Keypad7] = AK_NUMPAD7;
		m[kVK_ANSI_Keypad8] = AK_NUMPAD8;
		m[kVK_ANSI_Keypad9] = AK_NUMPAD9;
		m[kVK_Return] = AK_RETURN;
		m[kVK_Tab] = AK_TAB;
		m[kVK_Space] = AK_SPACE;
		m[kVK_Delete] = AK_BACK;
		m[kVK_Escape] = AK_ESCAPE;
		m[kVK_Shift] = AK_SHIFT;
		m[kVK_CapsLock] = AK_CAPITAL;
		m[kVK_Option] = AK_MENU;
		m[kVK_Control] = AK_CONTROL;
		m[kVK_RightShift] = AK_RSHIFT;
		m[kVK_RightOption] = AK_RMENU;
		m[kVK_RightControl] = AK_RCONTROL;
		m[kVK_F17] = AK_F17;
		m[kVK_VolumeUp] = AK_VOLUME_UP;
		m[kVK_VolumeDown] = AK_VOLUME_DOWN;
		m[kVK_Mute] = AK_VOLUME_MUTE;
		m[kVK_F18] = AK_F18;
		m[kVK_F19] = AK_F19;
		m[kVK_F20] = AK_F20;
		m[kVK_F5] = AK_F5;
		m[kVK_F6] = AK_F6;
		m[kVK_F7] = AK_F7;
		m[kVK_F3] = AK_F3;
		m[kVK_F8] = AK_F8;
		m[kVK_F9] = AK_F9;
		m[kVK_F11] = AK_F11;
		m[kVK_F13] = AK_F13;
		m[kVK_F16] = AK_F16;
		m[kVK_F14] = AK_F14;
		m[kVK_F10] = AK_F10;
		m[kVK_F12] = AK_F12;
		m[kVK_F15] = AK_F15;
		m[kVK_Help] = AK_HELP;
		m[kVK_Home] = AK_HOME;
		m[kVK_PageUp] = AK_PRIOR;
		m[kVK_ForwardDelete] = AK_DELETE;
		m[kVK_F4] = AK_F4;
		m[kVK_End] = AK_END;
		m[kVK_F2] = AK_F2;
		m[kVK_PageDown] = AK_NEXT;
		m[kVK_F1] = AK_F1;
		m[kVK_LeftArrow] = AK_LEFT;
		m[kVK_RightArrow] = AK_RIGHT;
		m[kVK_DownArrow] = AK_DOWN;
		m[kVK_UpArrow] = AK_UP;
//		m[kVK_ISO_Section] = AK_;
//		m[kVK_JIS_Yen] = AK_;
//		m[kVK_JIS_Underscore] = AK_;
//		m[kVK_JIS_KeypadComma] = AK_;
//		m[kVK_JIS_Eisu] = AK_;
//		m[kVK_JIS_Kana] = AK_;
//		m[kVK_Function] = AK_;
//		m[kVK_ANSI_Equal] = AK_;
//		m[kVK_ANSI_Minus] = AK_;
//		m[kVK_ANSI_RightBracket] = AK_;
//		m[kVK_ANSI_LeftBracket] = AK_;
//		m[kVK_ANSI_Quote] = AK_;
//		m[kVK_ANSI_Semicolon] = AK_;
//		m[kVK_ANSI_Backslash] = AK_;
//		m[kVK_ANSI_Comma] = AK_;
//		m[kVK_ANSI_Slash] = AK_;
//		m[kVK_ANSI_Period] = AK_;
//		m[kVK_ANSI_Grave] = AK_;
//		m[kVK_ANSI_KeypadDecimal] = AK_;
//		m[kVK_ANSI_KeypadMultiply] = AK_;
//		m[kVK_ANSI_KeypadPlus] = AK_;
//		m[kVK_ANSI_KeypadClear] = AK_;
//		m[kVK_ANSI_KeypadDivide] = AK_;
//		m[kVK_ANSI_KeypadEnter] = AK_;
//		m[kVK_ANSI_KeypadMinus] = AK_;
//		m[kVK_ANSI_KeypadEquals] = AK_;
		m[kVK_Command] = AK_LCOMMAND;
		m[0x36] = AK_RCOMMAND;
	}
}
