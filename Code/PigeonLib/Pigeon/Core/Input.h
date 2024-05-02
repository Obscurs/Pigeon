#pragma once

#include "Pigeon/Core/Core.h"

namespace pig 
{
	class PIGEON_API PlatformInput
	{
	public:
		inline static unsigned char GetKeyCode(unsigned char keyCode) { return s_Instance->GetKeyCodeImpl(keyCode); }
		inline static unsigned char GetMouseButtonCode(unsigned char keyCode) { return s_Instance->GetMouseButtonCodeImpl(keyCode); }

	protected:
		inline static bool IsKeyPressed(int keycode) { return s_Instance->IsKeyPressedImpl(keycode); }
		inline static bool IsMouseButtonPressed(int button) { return s_Instance->IsMouseButtonPressedImpl(button); }
		inline static std::pair<float, float> GetMousePosition() { return s_Instance->GetMousePositionImpl(); }
		inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
		inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); }
		inline static PlatformInput& GetInput() { return *s_Instance; }

		virtual bool IsKeyPressedImpl(int keycode) = 0;
		virtual bool IsMouseButtonPressedImpl(int button) = 0;
		virtual std::pair<float, float> GetMousePositionImpl() = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;
		virtual unsigned char GetKeyCodeImpl(unsigned char keyCode) = 0;
		virtual unsigned char GetMouseButtonCodeImpl(unsigned char keyCode) = 0;
	private:
		static pig::S_Ptr<PlatformInput> s_Instance;
	};
}