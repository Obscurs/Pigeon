#include "pch.h"
#include "WindowsInput.h"

#include "Pigeon/Application.h"
#include "Platform/Windows/WindowsWindow.h"

pig::Input* pig::Input::s_Instance = new pig::WindowsInput();

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