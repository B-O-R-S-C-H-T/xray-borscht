#include "input_mapping.h"

#include <array>
#include <dinput.h>

namespace InputMapping {

	ImGuiKey GetImGuiKeyFromDIK(int dik) {
		static constexpr std::array<ImGuiKey, 0x100> mapping = []() constexpr
		{
			std::array<ImGuiKey, 0x100> key{};
			for (auto& element : key) {
				element = ImGuiKey_None;
			}
			
			// Скопировано из dinput.h
			key[DIK_ESCAPE]       = ImGuiKey_Escape;
			key[DIK_1]            = ImGuiKey_1;
			key[DIK_2]            = ImGuiKey_2;
			key[DIK_3]            = ImGuiKey_3;
			key[DIK_4]            = ImGuiKey_4;
			key[DIK_5]            = ImGuiKey_5;
			key[DIK_6]            = ImGuiKey_6;
			key[DIK_7]            = ImGuiKey_7;
			key[DIK_8]            = ImGuiKey_8;
			key[DIK_9]            = ImGuiKey_9;
			key[DIK_0]            = ImGuiKey_0;
			key[DIK_MINUS]        = ImGuiKey_Minus;
			key[DIK_EQUALS]       = ImGuiKey_Equal;
			key[DIK_BACK]         = ImGuiKey_Backspace;
			key[DIK_TAB]          = ImGuiKey_Tab;
			key[DIK_Q]            = ImGuiKey_Q;
			key[DIK_W]            = ImGuiKey_W;
			key[DIK_E]            = ImGuiKey_E;
			key[DIK_R]            = ImGuiKey_R;
			key[DIK_T]            = ImGuiKey_T;
			key[DIK_Y]            = ImGuiKey_Y;
			key[DIK_U]            = ImGuiKey_U;
			key[DIK_I]            = ImGuiKey_I;
			key[DIK_O]            = ImGuiKey_O;
			key[DIK_P]            = ImGuiKey_P;
			key[DIK_LBRACKET]     = ImGuiKey_LeftBracket;
			key[DIK_RBRACKET]     = ImGuiKey_RightBracket;
			key[DIK_RETURN]       = ImGuiKey_Enter;
			key[DIK_A]            = ImGuiKey_A;
			key[DIK_S]            = ImGuiKey_S;
			key[DIK_D]            = ImGuiKey_D;
			key[DIK_F]            = ImGuiKey_F;
			key[DIK_G]            = ImGuiKey_G;
			key[DIK_H]            = ImGuiKey_H;
			key[DIK_J]            = ImGuiKey_J;
			key[DIK_K]            = ImGuiKey_K;
			key[DIK_L]            = ImGuiKey_L;
			key[DIK_SEMICOLON]    = ImGuiKey_Semicolon;
			key[DIK_APOSTROPHE]   = ImGuiKey_Apostrophe;
			key[DIK_GRAVE]        = ImGuiKey_GraveAccent;
			key[DIK_BACKSLASH]    = ImGuiKey_Backslash;
			key[DIK_Z]            = ImGuiKey_Z;
			key[DIK_X]            = ImGuiKey_X;
			key[DIK_C]            = ImGuiKey_C;
			key[DIK_V]            = ImGuiKey_V;
			key[DIK_B]            = ImGuiKey_B;
			key[DIK_N]            = ImGuiKey_N;
			key[DIK_M]            = ImGuiKey_M;
			key[DIK_COMMA]        = ImGuiKey_Comma;
			key[DIK_PERIOD]       = ImGuiKey_Period;
			key[DIK_SLASH]        = ImGuiKey_Slash;
			key[DIK_SPACE]        = ImGuiKey_Space;

			key[DIK_LCONTROL]     = ImGuiKey_LeftCtrl;
			key[DIK_LSHIFT]       = ImGuiKey_LeftShift;
			key[DIK_LMENU]        = ImGuiKey_LeftAlt;
			key[DIK_RCONTROL]     = ImGuiKey_RightCtrl;
			key[DIK_RSHIFT]       = ImGuiKey_RightShift;
			key[DIK_RMENU]        = ImGuiKey_RightAlt;
			key[DIK_CAPITAL]      = ImGuiKey_CapsLock;

			key[DIK_F1]           = ImGuiKey_F1;
			key[DIK_F2]           = ImGuiKey_F2;
			key[DIK_F3]           = ImGuiKey_F3;
			key[DIK_F4]           = ImGuiKey_F4;
			key[DIK_F5]           = ImGuiKey_F5;
			key[DIK_F6]           = ImGuiKey_F6;
			key[DIK_F7]           = ImGuiKey_F7;
			key[DIK_F8]           = ImGuiKey_F8;
			key[DIK_F9]           = ImGuiKey_F9;
			key[DIK_F10]          = ImGuiKey_F10;
			key[DIK_F11]          = ImGuiKey_F11;
			key[DIK_F12]          = ImGuiKey_F12;
			key[DIK_F13]          = ImGuiKey_None;
			key[DIK_F14]          = ImGuiKey_None;
			key[DIK_F15]          = ImGuiKey_None;
			key[DIK_NUMLOCK]      = ImGuiKey_NumLock;
			key[DIK_SCROLL]       = ImGuiKey_ScrollLock;

			key[DIK_NUMPAD7]      = ImGuiKey_Keypad7;
			key[DIK_NUMPAD8]      = ImGuiKey_Keypad8;
			key[DIK_NUMPAD9]      = ImGuiKey_Keypad9;
			key[DIK_SUBTRACT]     = ImGuiKey_KeypadSubtract;
			key[DIK_NUMPAD4]      = ImGuiKey_Keypad4;
			key[DIK_NUMPAD5]      = ImGuiKey_Keypad5;
			key[DIK_NUMPAD6]      = ImGuiKey_Keypad6;
			key[DIK_ADD]          = ImGuiKey_KeypadAdd;
			key[DIK_NUMPAD1]      = ImGuiKey_Keypad1;
			key[DIK_NUMPAD2]      = ImGuiKey_Keypad2;
			key[DIK_NUMPAD3]      = ImGuiKey_Keypad3;
			key[DIK_NUMPAD0]      = ImGuiKey_Keypad0;
			key[DIK_DECIMAL]      = ImGuiKey_KeypadDecimal;
			key[DIK_MULTIPLY]     = ImGuiKey_KeypadMultiply;
			key[DIK_DIVIDE]       = ImGuiKey_KeypadDivide;
			key[DIK_NUMPADENTER]  = ImGuiKey_KeypadEnter;
			key[DIK_NUMPADCOMMA]  = ImGuiKey_None;
			key[DIK_NUMPADEQUALS] = ImGuiKey_None;

			key[DIK_INSERT]       = ImGuiKey_Insert;
			key[DIK_DELETE]       = ImGuiKey_Delete;
			key[DIK_HOME]         = ImGuiKey_Home;
			key[DIK_END]          = ImGuiKey_End;
			key[DIK_PRIOR]        = ImGuiKey_PageUp;
			key[DIK_NEXT]         = ImGuiKey_PageDown;
			key[DIK_LEFT]         = ImGuiKey_LeftArrow;
			key[DIK_RIGHT]        = ImGuiKey_RightArrow;
			key[DIK_UP]           = ImGuiKey_UpArrow;
			key[DIK_DOWN]         = ImGuiKey_DownArrow;
			key[DIK_PAUSE]        = ImGuiKey_Pause;
			key[DIK_SYSRQ]        = ImGuiKey_PrintScreen;
			key[DIK_APPS]         = ImGuiKey_Menu;
			key[DIK_LWIN]         = ImGuiKey_LeftSuper;
			key[DIK_RWIN]         = ImGuiKey_RightSuper;

			key[DIK_KANA]         = ImGuiKey_None;
			key[DIK_ABNT_C1]      = ImGuiKey_None;
			key[DIK_CONVERT]      = ImGuiKey_None;
			key[DIK_NOCONVERT]    = ImGuiKey_None;
			key[DIK_YEN]          = ImGuiKey_None;
			key[DIK_ABNT_C2]      = ImGuiKey_None;
			key[DIK_PREVTRACK]    = ImGuiKey_None;
			key[DIK_AT]           = ImGuiKey_None;
			key[DIK_COLON]        = ImGuiKey_None;
			key[DIK_UNDERLINE]    = ImGuiKey_None;
			key[DIK_KANJI]        = ImGuiKey_None;
			key[DIK_STOP]         = ImGuiKey_None;
			key[DIK_AX]           = ImGuiKey_None;
			key[DIK_UNLABELED]    = ImGuiKey_None;
			key[DIK_NEXTTRACK]    = ImGuiKey_None;
			key[DIK_OEM_102]      = ImGuiKey_Backslash;

			key[DIK_MUTE]         = ImGuiKey_None;
			key[DIK_CALCULATOR]   = ImGuiKey_None;
			key[DIK_PLAYPAUSE]    = ImGuiKey_None;
			key[DIK_MEDIASTOP]    = ImGuiKey_None;
			key[DIK_VOLUMEDOWN]   = ImGuiKey_None;
			key[DIK_VOLUMEUP]     = ImGuiKey_None;
			key[DIK_WEBHOME]      = ImGuiKey_None;
			key[DIK_WEBSEARCH]    = ImGuiKey_None;
			key[DIK_WEBFAVORITES] = ImGuiKey_None;
			key[DIK_WEBREFRESH]   = ImGuiKey_None;
			key[DIK_WEBSTOP]      = ImGuiKey_None;
			key[DIK_WEBFORWARD]   = ImGuiKey_None;
			key[DIK_WEBBACK]      = ImGuiKey_None;
			key[DIK_MYCOMPUTER]   = ImGuiKey_None;
			key[DIK_MAIL]         = ImGuiKey_None;
			key[DIK_MEDIASELECT]  = ImGuiKey_None;
			key[DIK_POWER]        = ImGuiKey_None;
			key[DIK_SLEEP]        = ImGuiKey_None;
			key[DIK_WAKE]         = ImGuiKey_None;

			return key;
		}();

		if (dik >= 0 && static_cast<size_t>(dik) < mapping.size()) {
			ImGuiKey result = mapping[dik];
			if (result == ImGuiKey_None) {
#ifdef _DEBUG
				Msg("Unknown DIK_0x%02X", dik);
#endif
			}
			return result;
		}
		return ImGuiKey_None;
	}

} // namespace InputMapping
