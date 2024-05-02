#include "pch.h"
#include "WindowsInput.h"

#include "Pigeon/Core/Application.h"
#include "Pigeon/Core/KeyCodes.h"
#include "Platform/Windows/WindowsWindow.h"
#include "Platform/Windows/WindowsInputKeyCodeMapping.h"

#ifndef TESTS_ENABLED
pig::S_Ptr<pig::PlatformInput> pig::PlatformInput::s_Instance = std::make_shared<pig::WindowsInput>();
#endif
bool pig::WindowsInput::IsKeyPressedImpl(int keycode)
{
	return (GetAsyncKeyState(keycode) & 0x8000) != 0;
}

bool pig::WindowsInput::IsMouseButtonPressedImpl(int button)
{
	return (GetAsyncKeyState(button) & 0x8000) != 0;
}

std::pair<float, float> pig::WindowsInput::GetMousePositionImpl()
{
	auto window = static_cast<HWND>(pig::Application::Get().GetWindow().GetNativeWindow());

	POINT point;
	if (ScreenToClient(window, &point)) //Use GetCursorPos(&point) if you want to get the global pos instead
	{
		return { static_cast<float>(point.x), static_cast<float>(point.y) };
	}

	// Return some default value if unable to get position
	return { 0.0f, 0.0f };
}

float pig::WindowsInput::GetMouseXImpl()
{
	auto[x, y] = GetMousePositionImpl();
	return x;
}

float pig::WindowsInput::GetMouseYImpl()
{
	auto[x, y] = GetMousePositionImpl();
	return y;
}

unsigned char pig::WindowsInput::GetMouseButtonCodeImpl(unsigned char keyCode)
{
	switch (keyCode)
	{
	case WIN_PG_MOUSE_BUTTON_LEFT : return PG_MOUSE_BUTTON_LEFT;
	case WIN_PG_MOUSE_BUTTON_RIGHT : return PG_MOUSE_BUTTON_RIGHT;
	case WIN_PG_MOUSE_BUTTON_MIDDLE : return PG_MOUSE_BUTTON_MIDDLE;
	case WIN_PG_MOUSE_BUTTON_4 : return PG_MOUSE_BUTTON_4;
	case WIN_PG_MOUSE_BUTTON_5 : return PG_MOUSE_BUTTON_5;
	case WIN_PG_MOUSE_BUTTON_6 : return PG_MOUSE_BUTTON_6;
	case WIN_PG_MOUSE_BUTTON_7 : return PG_MOUSE_BUTTON_7;
	case WIN_PG_MOUSE_BUTTON_8 : return PG_MOUSE_BUTTON_8;
	default: 
	{
		PG_CORE_ASSERT(false, "Type not found");
		return 0;
	}
	}
}

unsigned char pig::WindowsInput::GetKeyCodeImpl(unsigned char keyCode)
{
	switch (keyCode)
	{
	case WIN_PG_KEY_0: return PG_KEY_0;
	case WIN_PG_KEY_1: return PG_KEY_1;
	case WIN_PG_KEY_2: return PG_KEY_2;
	case WIN_PG_KEY_3: return PG_KEY_3;
	case WIN_PG_KEY_4: return PG_KEY_4;
	case WIN_PG_KEY_5: return PG_KEY_5;
	case WIN_PG_KEY_6: return PG_KEY_6;
	case WIN_PG_KEY_7: return PG_KEY_7;
	case WIN_PG_KEY_8: return PG_KEY_8;
	case WIN_PG_KEY_9: return PG_KEY_9;
	case WIN_PG_KEY_A: return PG_KEY_A;
	case WIN_PG_KEY_B: return PG_KEY_B;
	case WIN_PG_KEY_C: return PG_KEY_C;
	case WIN_PG_KEY_D: return PG_KEY_D;
	case WIN_PG_KEY_E: return PG_KEY_E;
	case WIN_PG_KEY_F: return PG_KEY_F;
	case WIN_PG_KEY_G: return PG_KEY_G;
	case WIN_PG_KEY_H: return PG_KEY_H;
	case WIN_PG_KEY_I: return PG_KEY_I;
	case WIN_PG_KEY_J: return PG_KEY_J;
	case WIN_PG_KEY_K: return PG_KEY_K;
	case WIN_PG_KEY_L: return PG_KEY_L;
	case WIN_PG_KEY_M: return PG_KEY_M;
	case WIN_PG_KEY_N: return PG_KEY_N;
	case WIN_PG_KEY_O: return PG_KEY_O;
	case WIN_PG_KEY_P: return PG_KEY_P;
	case WIN_PG_KEY_Q: return PG_KEY_Q;
	case WIN_PG_KEY_R: return PG_KEY_R;
	case WIN_PG_KEY_S: return PG_KEY_S;
	case WIN_PG_KEY_T: return PG_KEY_T;
	case WIN_PG_KEY_U: return PG_KEY_U;
	case WIN_PG_KEY_V: return PG_KEY_V;
	case WIN_PG_KEY_W: return PG_KEY_W;
	case WIN_PG_KEY_X: return PG_KEY_X;
	case WIN_PG_KEY_Y: return PG_KEY_Y;
	case WIN_PG_KEY_Z: return PG_KEY_Z;
	case WIN_PG_KEY_SPACE: return PG_KEY_SPACE;
	case WIN_PG_KEY_ESCAPE: return PG_KEY_ESCAPE;
	case WIN_PG_KEY_ENTER: return PG_KEY_ENTER;
	case WIN_PG_KEY_TAB: return PG_KEY_TAB;
	case WIN_PG_KEY_BACKSPACE: return PG_KEY_BACKSPACE;
	case WIN_PG_KEY_INSERT: return PG_KEY_INSERT;
	case WIN_PG_KEY_DELETE: return PG_KEY_DELETE;
	case WIN_PG_KEY_RIGHT: return PG_KEY_RIGHT;
	case WIN_PG_KEY_LEFT: return PG_KEY_LEFT;
	case WIN_PG_KEY_DOWN: return PG_KEY_DOWN;
	case WIN_PG_KEY_UP: return PG_KEY_UP;
	case WIN_PG_KEY_PAGE_UP: return PG_KEY_PAGE_UP;
	case WIN_PG_KEY_PAGE_DOWN: return PG_KEY_PAGE_DOWN;
	case WIN_PG_KEY_HOME: return PG_KEY_HOME;
	case WIN_PG_KEY_END: return PG_KEY_END;
	case WIN_PG_KEY_CAPS_LOCK: return PG_KEY_CAPS_LOCK;
	case WIN_PG_KEY_CONTROL: return PG_KEY_CONTROL;
	case WIN_PG_KEY_ALT: return PG_KEY_ALT;
	case WIN_PG_KEY_SHIFT: return PG_KEY_SHIFT;
	case WIN_PG_KEY_PAUSE: return PG_KEY_PAUSE;
	case WIN_PG_KEY_F1: return PG_KEY_F1;
	case WIN_PG_KEY_F2: return PG_KEY_F2;
	case WIN_PG_KEY_F3: return PG_KEY_F3;
	case WIN_PG_KEY_F4: return PG_KEY_F4;
	case WIN_PG_KEY_F5: return PG_KEY_F5;
	case WIN_PG_KEY_F6: return PG_KEY_F6;
	case WIN_PG_KEY_F7: return PG_KEY_F7;
	case WIN_PG_KEY_F8: return PG_KEY_F8;
	case WIN_PG_KEY_F9: return PG_KEY_F9;
	case WIN_PG_KEY_F10: return PG_KEY_F10;
	case WIN_PG_KEY_F11: return PG_KEY_F11;
	case WIN_PG_KEY_F12: return PG_KEY_F12;
	case WIN_PG_KEY_F13: return PG_KEY_F13;
	case WIN_PG_KEY_F14: return PG_KEY_F14;
	case WIN_PG_KEY_F15: return PG_KEY_F15;
	case WIN_PG_KEY_F16: return PG_KEY_F16;
	case WIN_PG_KEY_F17: return PG_KEY_F17;
	case WIN_PG_KEY_F18: return PG_KEY_F18;
	case WIN_PG_KEY_F19: return PG_KEY_F19;
	case WIN_PG_KEY_F20: return PG_KEY_F20;
	case WIN_PG_KEY_F21: return PG_KEY_F21;
	case WIN_PG_KEY_F22: return PG_KEY_F22;
	case WIN_PG_KEY_F23: return PG_KEY_F23;
	case WIN_PG_KEY_F24: return PG_KEY_F24;
	case WIN_PG_KEY_KP_0: return PG_KEY_KP_0;
	case WIN_PG_KEY_KP_1: return PG_KEY_KP_1;
	default:
	{
		PG_CORE_ASSERT(false, "Type not found");
		return 0;
	}
	}
}
