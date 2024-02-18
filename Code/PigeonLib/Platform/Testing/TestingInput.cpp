#include "pch.h"
#include "TestingInput.h"

#ifdef TESTS_ENABLED
pig::S_Ptr<pig::Input> pig::Input::s_Instance = std::make_shared<pig::TestingInput>();
#endif

bool pig::TestingInput::IsKeyPressedImpl(int keycode)
{
	for (int code : TESTING_KeysPressed)
	{
		if (code == keycode)
			return true;
	}
	return false;
}

bool pig::TestingInput::IsMouseButtonPressedImpl(int button)
{
	for (int code : TESTING_MouseButtonKeysPressed)
	{
		if (code == button)
			return true;
	}
	return false;
}

std::pair<float, float> pig::TestingInput::GetMousePositionImpl()
{
	return TESTING_MousePos;
}

float pig::TestingInput::GetMouseXImpl()
{
	return TESTING_MousePos.first;
}

float pig::TestingInput::GetMouseYImpl()
{
	return TESTING_MousePos.second;
}