#pragma once
#include "Pigeon/Input.h"

namespace pig 
{
	class TestingInput : public Input
	{
	protected:
		virtual bool IsKeyPressedImpl(int keycode) override;

		virtual bool IsMouseButtonPressedImpl(int button) override;
		virtual std::pair<float, float> GetMousePositionImpl() override;
		virtual float GetMouseXImpl() override;
		virtual float GetMouseYImpl() override;

	public:
		std::vector<int> TESTING_KeysPressed;
		std::vector<int> TESTING_MouseButtonKeysPressed;
		std::pair<float, float> TESTING_MousePos;
	};
}