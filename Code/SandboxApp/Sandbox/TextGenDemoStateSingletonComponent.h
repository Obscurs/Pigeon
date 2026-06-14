#pragma once

#include <cstring>

namespace sbx
{
	// The editable state of the text-generation demo panel, owned by TextGenDemoSystem (like the window
	// panel's selection singleton). Seeded once with demo defaults; thereafter the user owns it via the
	// ImGui controls until the next Generate. The prompt is a fixed char buffer so ImGui::InputTextMultiline
	// can edit it in place without pulling in imgui_stdlib (not compiled in this project).
	struct TextGenDemoStateSingletonComponent
	{
		TextGenDemoStateSingletonComponent()
		{
			const char* defaultPrompt = "Write a short adventure intro about a pigeon knight.";
			std::strncpy(m_Prompt, defaultPrompt, sizeof(m_Prompt) - 1);
			m_Prompt[sizeof(m_Prompt) - 1] = '\0';
		}

		char m_Prompt[1024] = {};
		float m_Temperature = 0.8f;
		int m_MaxTokens = 256;
	};
}
