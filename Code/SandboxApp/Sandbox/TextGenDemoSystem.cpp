#include "Sandbox/TextGenDemoSystem.h"

#include "Pigeon/ECS/World.h"
#include "Pigeon/TextGen/GenerateTextRequestOneFrameComponent.h"
#include "Pigeon/TextGen/TextGenJobSingletonComponent.h"
#include "Pigeon/TextGen/TextGenResultSingletonComponent.h"
#include "Sandbox/TextGenDemoIds.h"
#include "Sandbox/TextGenDemoStateSingletonComponent.h"

#include <imgui.h>

pg::SystemAccessDecl sbx::TextGenDemoSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::TextGenResultSingletonComponent)),
		std::type_index(typeid(pg::TextGenJobSingletonComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(sbx::TextGenDemoStateSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(sbx::TextGenDemoStateSingletonComponent)),
		std::type_index(typeid(pg::GenerateTextRequestOneFrameComponent)),
	};
	return decl;
}

void sbx::TextGenDemoSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	// The engine TextGen singletons are created lazily by TextGenSystem; wait until they exist.
	auto jobView = accessor.View<const pg::TextGenJobSingletonComponent>();
	auto resultView = accessor.View<const pg::TextGenResultSingletonComponent>();
	if (jobView.empty() || resultView.empty())
	{
		return;
	}

	// Seed the editable panel state once (deferred -> visible next frame); thereafter the user owns it.
	auto stateView = accessor.View<sbx::TextGenDemoStateSingletonComponent>();
	if (stateView.empty())
	{
		accessor.EmplaceDeferred<sbx::TextGenDemoStateSingletonComponent>(accessor.Create(), sbx::TextGenDemoStateSingletonComponent{});
		return;
	}

	// The test build pushes no ImGuiLayer, so no ImGui context exists there; guard every call.
	if (ImGui::GetCurrentContext() == nullptr)
	{
		return;
	}

	sbx::TextGenDemoStateSingletonComponent& state = stateView.get<sbx::TextGenDemoStateSingletonComponent>(stateView.front());
	const pg::TextGenJobSingletonComponent& job = jobView.get<const pg::TextGenJobSingletonComponent>(jobView.front());
	const pg::TextGenResultSingletonComponent& results = resultView.get<const pg::TextGenResultSingletonComponent>(resultView.front());
	const bool busy = job.m_ActiveJob != nullptr;

	ImGui::Begin("Text Generation");
	ImGui::TextWrapped("Prompt:");
	ImGui::InputTextMultiline("##prompt", state.m_Prompt, sizeof(state.m_Prompt), ImVec2(-1.0f, ImGui::GetTextLineHeight() * 4));
	ImGui::SliderFloat("Temperature", &state.m_Temperature, 0.0f, 2.0f);
	ImGui::SliderInt("Max tokens", &state.m_MaxTokens, 16, 1024);

	bool generate = false;
	if (busy)
	{
		ImGui::BeginDisabled();
		ImGui::Button("Generating...");
		ImGui::EndDisabled();
	}
	else
	{
		generate = ImGui::Button("Generate");
	}

	ImGui::Separator();
	ImGui::TextWrapped("Result:");
	std::unordered_map<pg::UUID, std::string>::const_iterator it = results.m_Results.find(sbx::k_TextGenTargetID);
	if (it != results.m_Results.end() && !it->second.empty())
	{
		ImGui::TextWrapped("%s", it->second.c_str());
	}
	else if (busy)
	{
		ImGui::TextWrapped("(generating, please wait...)");
	}
	else
	{
		ImGui::TextWrapped("(none yet - press Generate)");
	}
	ImGui::End();

	if (generate)
	{
		pg::GenerateTextRequestOneFrameComponent request;
		request.m_TargetTextID = sbx::k_TextGenTargetID;
		request.m_Prompt = state.m_Prompt;
		request.m_MaxTokens = state.m_MaxTokens;
		request.m_Temperature = state.m_Temperature;
		accessor.EmplaceOneframe<pg::GenerateTextRequestOneFrameComponent>(accessor.Create(), std::move(request));
	}
}
