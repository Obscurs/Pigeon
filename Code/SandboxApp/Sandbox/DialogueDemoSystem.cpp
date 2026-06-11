#include "Sandbox/DialogueDemoSystem.h"

#include "Pigeon/ECS/World.h"
#include "Pigeon/UI/UIComponents.h"
#include "Sandbox/DialogueDemoSingletonComponent.h"
#include "Sandbox/SandboxConfigSingletonComponent.h"
#include "Sandbox/UIElementHelpers.h"

#include <algorithm>
#include <string>

#include <imgui.h>

pg::SystemAccessDecl sbx::DialogueDemoSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(sbx::SandboxConfigSingletonComponent)),
		std::type_index(typeid(pg::ui::BaseComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(sbx::DialogueDemoSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(sbx::DialogueDemoSingletonComponent)),
		std::type_index(typeid(pg::ui::UIUpdateTextRevealOneFrameComponent)),
	};
	return decl;
}

void sbx::DialogueDemoSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto configView = accessor.View<const sbx::SandboxConfigSingletonComponent>();
	if (configView.empty())
	{
		return;
	}
	const sbx::SandboxConfigSingletonComponent& config = configView.get<const sbx::SandboxConfigSingletonComponent>(configView.front());

	// The demo state lives in ECS (like the other debug panels). Seed it once with the default line.
	auto demoView = accessor.View<sbx::DialogueDemoSingletonComponent>();
	if (demoView.empty())
	{
		pg::ecs::Entity ent = accessor.Create();
		accessor.EmplaceDeferred<sbx::DialogueDemoSingletonComponent>(ent);
		return;
	}
	sbx::DialogueDemoSingletonComponent& demo = demoView.get<sbx::DialogueDemoSingletonComponent>(demoView.front());

	// The ImGui widget is just one way to edit the buffer/speed; it is absent in the test build, so the
	// reveal logic below runs regardless of any ImGui context.
	if (ImGui::GetCurrentContext() != nullptr)
	{
		ImGui::Begin("Dialogue");
		ImGui::TextUnformatted("Types out in the nine-slice panel. Edit to replay.");
		ImGui::InputTextMultiline("##dialogue", demo.m_TextBuffer, sizeof(demo.m_TextBuffer), ImVec2(0.f, 60.f));
		ImGui::SliderFloat("Chars / sec", &demo.m_CharsPerSecond, 1.f, 120.f);
		if (ImGui::Button("Replay"))
		{
			demo.m_RevealedChars = 0.f;
			demo.m_LastText.clear();
		}
		ImGui::End();
	}

	// Restart the reveal whenever the line changes (an ImGui edit, or the Replay button clearing it).
	const std::string currentText(demo.m_TextBuffer);
	if (currentText != demo.m_LastText)
	{
		demo.m_LastText = currentText;
		demo.m_RevealedChars = 0.f;
	}

	// Advance the reveal, clamped so it never runs past the end of the line.
	const float maxChars = static_cast<float>(currentText.size());
	demo.m_RevealedChars = std::min(demo.m_RevealedChars + demo.m_CharsPerSecond * ts.AsSeconds(), maxChars);

	pg::ecs::Entity dialogueEnt = sbx::FindUIElementByUUID(accessor, config.m_DialogueTextID);
	if (dialogueEnt == pg::ecs::null)
	{
		return;
	}

	pg::ui::UIUpdateTextRevealOneFrameComponent reveal;
	reveal.m_Text = currentText;
	reveal.m_VisibleChars = static_cast<int>(demo.m_RevealedChars);
	accessor.EmplaceOneframe<pg::ui::UIUpdateTextRevealOneFrameComponent>(dialogueEnt, std::move(reveal));
}
