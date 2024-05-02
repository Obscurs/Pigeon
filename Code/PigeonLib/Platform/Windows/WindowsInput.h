#pragma once
#include "Pigeon/Core/Input.h"

namespace pig 
{
	class WindowsInput : public PlatformInput
	{
	protected:
		virtual bool IsKeyPressedImpl(int keycode) override;
		virtual bool IsMouseButtonPressedImpl(int button) override;
		virtual std::pair<float, float> GetMousePositionImpl() override;
		virtual float GetMouseXImpl() override;
		virtual float GetMouseYImpl() override;
		virtual unsigned char GetKeyCodeImpl(unsigned char keyCode) override;
		virtual unsigned char GetMouseButtonCodeImpl(unsigned char keyCode) override;
	};
}