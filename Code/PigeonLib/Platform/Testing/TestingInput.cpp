#include "pch.h"
#include "Platform/Testing/TestingInput.h"

#ifdef TESTS_ENABLED
pg::S_Ptr<pg::PlatformInput> pg::PlatformInput::s_Instance = std::make_shared<pg::TestingInput>();
#endif

bool pg::TestingInput::IsKeyPressedImpl(int keycode)
{
	for (int code : TESTING_KeysPressed)
	{
		if (code == keycode)
			return true;
	}
	return false;
}

bool pg::TestingInput::IsMouseButtonPressedImpl(int button)
{
	for (int code : TESTING_MouseButtonKeysPressed)
	{
		if (code == button)
			return true;
	}
	return false;
}

std::pair<float, float> pg::TestingInput::GetMousePositionImpl()
{
	return TESTING_MousePos;
}

float pg::TestingInput::GetMouseXImpl()
{
	return TESTING_MousePos.first;
}

float pg::TestingInput::GetMouseYImpl()
{
	return TESTING_MousePos.second;
}

unsigned char pg::TestingInput::GetKeyCodeImpl(unsigned char keyCode)
{
	return keyCode;
}

unsigned char pg::TestingInput::GetMouseButtonCodeImpl(unsigned char keyCode)
{
	return keyCode;
}
