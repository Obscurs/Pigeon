
namespace pig
{
	struct InputStateSingletonComponent
	{
		InputStateSingletonComponent() = default;
		InputStateSingletonComponent(const InputStateSingletonComponent&) = default;

		std::unordered_map<int, int> m_KeysPressed;
		std::unordered_map<int, int> m_KeysReleased;
		std::vector<int> m_KeysTyped;
		glm::vec2 m_MousePos = glm::vec2(0.f, 0.f);
		glm::vec2 m_MouseScroll = glm::vec2(0.f, 0.f);
	};
}