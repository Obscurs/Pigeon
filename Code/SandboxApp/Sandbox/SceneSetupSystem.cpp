#include "Sandbox/SceneSetupSystem.h"

#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/OrthographicCameraComponent.h"
#include "Pigeon/Transform/TransformRequestData.h"
#include "Pigeon/UI/UIComponents.h"
#include "Sandbox/InputReadoutTagComponent.h"
#include "Sandbox/LabelComponent.h"
#include "Sandbox/SandboxConfigSingletonComponent.h"
#include "Sandbox/SceneReadySingletonComponent.h"
#include "Sandbox/SceneTransformRequestOneFrameComponent.h"
#include "Sandbox/SpriteComponent.h"

#include <glm/gtc/quaternion.hpp>

namespace
{
	// Requests a full world transform for a freshly created scene entity. Scene entities are static,
	// so the transform is set once at creation and never updated.
	void EmitSceneTransform(pg::CheckedRegistryAccessor& accessor, pg::ecs::Entity ent, const glm::vec3& position, const glm::vec3& scale)
	{
		sbx::SceneTransformRequestOneFrameComponent request;
		request.m_Data.m_SetPosition = true;
		request.m_Data.m_Position = position;
		request.m_Data.m_SetRotation = true;
		request.m_Data.m_Rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
		request.m_Data.m_SetScale = true;
		request.m_Data.m_Scale = scale;
		request.m_Data.m_SetBounds = true;
		request.m_Data.m_BoundsMin = glm::vec3(0.f, 0.f, 0.f);
		request.m_Data.m_BoundsMax = glm::vec3(1.f, 1.f, 0.f);
		accessor.EmplaceOneframe<sbx::SceneTransformRequestOneFrameComponent>(ent, std::move(request));
	}

	void CreateCamera(pg::CheckedRegistryAccessor& accessor)
	{
		pg::ecs::Entity ent = accessor.Create();
		pg::OrthographicCameraComponent camera;
		camera.m_AspectRatio = 1280.f / 720.f;
		camera.m_Camera = pg::OrthographicCamera(
			-camera.m_AspectRatio * camera.m_ZoomLevel, camera.m_AspectRatio * camera.m_ZoomLevel,
			-camera.m_ZoomLevel, camera.m_ZoomLevel);
		camera.m_ReactsToInput = true;
		accessor.EmplaceDeferred<pg::OrthographicCameraComponent>(ent, std::move(camera));

		// The camera location lives in its PositionComponent; CameraSystem pans it from here.
		sbx::SceneTransformRequestOneFrameComponent request;
		request.m_Data.m_SetPosition = true;
		request.m_Data.m_Position = glm::vec3(0.f, 0.f, 0.f);
		accessor.EmplaceOneframe<sbx::SceneTransformRequestOneFrameComponent>(ent, std::move(request));
	}

	void CreateSprite(pg::CheckedRegistryAccessor& accessor, const sbx::SandboxConfigSingletonComponent& config)
	{
		pg::ecs::Entity ent = accessor.Create();
		sbx::SpriteComponent sprite;
		// sampleSprite.png is a 9x5 grid of walk-cycle frames; show the top-left frame. The
		// renderer treats these coordinates as raw UVs, so they must be normalised to [0,1].
		sprite.m_TexCoordsRect = glm::vec4(0.f, 0.f, 1.f / 9.f, 1.f / 5.f);
		sprite.m_TextureID = config.m_SpriteTextureID;
		accessor.EmplaceDeferred<sbx::SpriteComponent>(ent, std::move(sprite));
		EmitSceneTransform(accessor, ent, glm::vec3(-1.5f, -0.9f, 0.f), glm::vec3(0.6f, 0.6f, 1.f));
	}

	pg::ecs::Entity CreateLabel(pg::CheckedRegistryAccessor& accessor, const glm::vec3& position, const glm::vec3& scale, const std::string& text, const pg::UUID& fontID, const glm::vec4& color)
	{
		pg::ecs::Entity ent = accessor.Create();
		sbx::LabelComponent label;
		label.m_Text = text;
		label.m_FontID = fontID;
		label.m_Color = color;
		label.m_Kerning = 0.1f;
		label.m_Linespacing = 0.1f;
		accessor.EmplaceDeferred<sbx::LabelComponent>(ent, std::move(label));
		EmitSceneTransform(accessor, ent, position, scale);
		return ent;
	}
}

pg::SystemAccessDecl sbx::SceneSetupSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(sbx::SandboxConfigSingletonComponent)),
		std::type_index(typeid(pg::ResourceMapSingletonComponent)),
		std::type_index(typeid(sbx::SceneReadySingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::OrthographicCameraComponent)),
		std::type_index(typeid(sbx::SpriteComponent)),
		std::type_index(typeid(sbx::LabelComponent)),
		std::type_index(typeid(sbx::SceneTransformRequestOneFrameComponent)),
		std::type_index(typeid(sbx::InputReadoutTagComponent)),
		std::type_index(typeid(pg::ui::LoadLayoutEvent)),
		std::type_index(typeid(sbx::SceneReadySingletonComponent)),
	};
	return decl;
}

void sbx::SceneSetupSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto configView = accessor.View<const sbx::SandboxConfigSingletonComponent>();
	auto resourcesView = accessor.View<const pg::ResourceMapSingletonComponent>();
	if (configView.empty() || resourcesView.empty())
	{
		return;
	}
	if (!accessor.View<const sbx::SceneReadySingletonComponent>().empty())
	{
		return;
	}
	const sbx::SandboxConfigSingletonComponent& config = configView.get<const sbx::SandboxConfigSingletonComponent>(configView.front());

	CreateCamera(accessor);
	CreateSprite(accessor, config);

	CreateLabel(accessor, glm::vec3(-1.7f, 1.5f, 0.f), glm::vec3(0.4f, 0.4f, 1.f),
		"Pigeon Engine Showcase", config.m_BoldFontID, glm::vec4(1.f, 0.9f, 0.2f, 1.f));
	CreateLabel(accessor, glm::vec3(-1.7f, 1.1f, 0.f), glm::vec3(0.18f, 0.18f, 1.f),
		"WASD pan   scroll zoom   SPACE spawn", config.m_DefaultFontID, glm::vec4(0.9f, 0.9f, 0.9f, 1.f));

	pg::ecs::Entity readout = CreateLabel(accessor, glm::vec3(-1.7f, -1.4f, 0.f), glm::vec3(0.16f, 0.16f, 1.f),
		"", config.m_DefaultFontID, glm::vec4(0.6f, 0.9f, 1.f, 1.f));
	accessor.EmplaceDeferred<sbx::InputReadoutTagComponent>(readout);

	pg::ui::LoadLayoutEvent layoutEvent;
	layoutEvent.m_UUID = config.m_MainLayoutID;
	accessor.EmplaceEvent<pg::ui::LoadLayoutEvent>(std::move(layoutEvent));

	pg::ecs::Entity readyEnt = accessor.Create();
	accessor.EmplaceDeferred<sbx::SceneReadySingletonComponent>(readyEnt);
}
