#include "pch.h"
#include "Pigeon/Core/WindowConfigSystem.h"

#include "Pigeon/Core/Application.h"
#include "Pigeon/Core/EWindowMode.h"
#include "Pigeon/Core/EngineConfigSingletonComponent.h"
#include "Pigeon/Core/FileUtils.h"
#include "Pigeon/Core/SetWindowResolutionRequestOneFrameComponent.h"
#include "Pigeon/Core/Window.h"
#include "Pigeon/Core/WindowConfigSingletonComponent.h"
#include "Pigeon/ECS/World.h"

namespace
{
	// Applies the resolution + mode to the live OS window. The window is a platform resource owned by
	// the Application host (not ECS), so it is reached through Application::Get() — the same path
	// WindowsInput / the DX11 layer / ImGuiLayer use. The Testing build has no Application, so this is
	// a no-op there.
	void ApplyToLiveWindow(const pg::WindowConfigSingletonComponent& wc)
	{
		if (!pg::Application::HasInstance())
		{
			return;
		}
		pg::Application::Get().GetWindow().ApplyWindowConfig(wc.m_Width, wc.m_Height, wc.m_Mode);
	}

	// Merges the window fields into the savedata Config.json, preserving any other override keys.
	void PersistWindowConfig(const std::string& savedataPath, const pg::WindowConfigSingletonComponent& wc)
	{
		if (savedataPath.empty())
		{
			return;
		}

		const std::string savedataConfigPath = savedataPath + "/Config.json";

		json jsonObject = json::object();
		if (pg::FileExists(savedataConfigPath))
		{
			jsonObject = json::parse(pg::ReadFileToString(savedataConfigPath));
		}

		jsonObject["windowWidth"] = wc.m_Width;
		jsonObject["windowHeight"] = wc.m_Height;
		jsonObject["windowMode"] = pg::WindowModeToString(wc.m_Mode);

		pg::WriteStringToFile(savedataConfigPath, jsonObject.dump(1, '\t') + "\n");
	}
}

pg::SystemAccessDecl pg::WindowConfigSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::EngineConfigSingletonComponent)),
		std::type_index(typeid(pg::SetWindowResolutionRequestOneFrameComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(pg::WindowConfigSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::WindowConfigSingletonComponent)),
	};
	return decl;
}

void pg::WindowConfigSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto windowConfigView = accessor.View<pg::WindowConfigSingletonComponent>();
	if (windowConfigView.empty())
	{
		auto configView = accessor.View<const pg::EngineConfigSingletonComponent>();
		if (configView.empty())
		{
			return;
		}
		const pg::EngineConfigSingletonComponent& config = configView.get<const pg::EngineConfigSingletonComponent>(configView.front());

		pg::WindowConfigSingletonComponent windowConfig;
		windowConfig.m_Width = config.m_WindowWidth;
		windowConfig.m_Height = config.m_WindowHeight;
		windowConfig.m_Mode = config.m_WindowMode;

		// Apply the configured resolution to the live window on startup. The savedata file already
		// reflects this value, so it is not rewritten here.
		ApplyToLiveWindow(windowConfig);

		pg::ecs::Entity ent = accessor.Create();
		accessor.EmplaceDeferred<pg::WindowConfigSingletonComponent>(ent, std::move(windowConfig));
		return;
	}

	pg::WindowConfigSingletonComponent& windowConfig = windowConfigView.get<pg::WindowConfigSingletonComponent>(windowConfigView.front());

	// Apply runtime resolution changes; the last request of the frame wins.
	bool changed = false;
	for (pg::ecs::Entity ent : accessor.View<const pg::SetWindowResolutionRequestOneFrameComponent>())
	{
		const pg::SetWindowResolutionRequestOneFrameComponent& request = accessor.Get<const pg::SetWindowResolutionRequestOneFrameComponent>(ent);
		windowConfig.m_Width = request.m_Width;
		windowConfig.m_Height = request.m_Height;
		windowConfig.m_Mode = request.m_Mode;
		changed = true;
	}

	if (!changed)
	{
		return;
	}

	ApplyToLiveWindow(windowConfig);

	auto configView = accessor.View<const pg::EngineConfigSingletonComponent>();
	if (!configView.empty())
	{
		const pg::EngineConfigSingletonComponent& config = configView.get<const pg::EngineConfigSingletonComponent>(configView.front());
		PersistWindowConfig(config.m_SavedataPath, windowConfig);
	}
}
