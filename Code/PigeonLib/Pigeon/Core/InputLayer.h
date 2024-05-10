#pragma once

#include "Pigeon/Core/Layer.h"

#include "Pigeon/Events/Event.h"

namespace pig
{
	class PIGEON_API Input
	{
	public:
		static bool IsKeyTyped(int keycode);
		static bool IsKeyPressed(int keycode, bool justPressed = false);
		static bool IsKeyReleased(int keycode);

		static bool IsMouseButtonPressed(int button, bool justPressed = false);
		static bool IsMouseButtonReleased(int button);

		static glm::vec2 GetMousePosition();
		static glm::vec2 GetMouseScrolled();

		static const std::vector<int>& GetKeysTyped();

		inline static Input& GetInput() { return *s_Instance; }

	private:
		static pig::S_Ptr<Input> s_Instance;
	};

	class PIGEON_API InputLayer : public Layer
	{
	public:
		InputLayer();
		~InputLayer() = default;

		virtual bool OnEvent(const pig::Event& e) override;
		virtual void OnUpdate(const pig::Timestep& ts) override;

	protected:
		bool IsKeyTyped(int keycode) const;
		bool IsKeyPressed(int keycode, bool justPressed = false) const;
		bool IsKeyReleased(int keycode) const;

		bool IsMouseButtonPressed(int button, bool justPressed = false) const;
		bool IsMouseButtonReleased(int button) const;

		glm::vec2 GetMousePosition() const;
		glm::vec2 GetMouseScrolled() const;

		const std::vector<int>& GetKeysTyped() const;

		friend class Input;
	private:
		struct InputEvent
		{
			pig::EventType m_Type = pig::EventType::None;
			int m_KeyCode = 0;
			float m_FloatData1 = 0;
			float m_FloatData2 = 0;
		};

		bool AppendKeyEvent(const pig::Event& e);
		bool AppendKeyTypedEvent(const pig::Event& e);
		bool AppendMouseMoveEvent(const pig::Event& e);
		bool AppendMouseButtonEvent(const pig::Event& e);

		void ProcessEvents();

		std::vector<InputEvent> m_Events;

		std::unordered_map<int, int> m_KeysPressed;
		std::unordered_map<int, int> m_KeysReleased;
		std::vector<int> m_KeysTyped;
		glm::vec2 m_MousePos = glm::vec2(0.f, 0.f);
		glm::vec2 m_MouseScroll = glm::vec2(0.f, 0.f);
	};
}