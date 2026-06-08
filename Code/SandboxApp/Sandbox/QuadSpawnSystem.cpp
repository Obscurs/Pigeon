#include "Sandbox/QuadSpawnSystem.h"

#include "Pigeon/Core/InputStateSingletonComponent.h"
#include "Pigeon/Core/KeyCodes.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Transform/TransformRequestData.h"
#include "Sandbox/LifetimeComponent.h"
#include "Sandbox/QuadComponent.h"
#include "Sandbox/QuadSpawnTransformRequestOneFrameComponent.h"
#include "Sandbox/SandboxConfigSingletonComponent.h"
#include "Sandbox/SpawnerSingletonComponent.h"
#include "Sandbox/SpinComponent.h"

#include <glm/gtc/quaternion.hpp>

namespace
{
	struct QuadSpec
	{
		glm::vec3 m_Anchor{ 0.f };
		glm::vec3 m_Scale{ 1.f };
		glm::vec3 m_BaseColor{ 1.f };
		pg::UUID m_TextureID;
		float m_RotationSpeed{ 0.f };
		float m_OrbitRadius{ 0.f };
		float m_OrbitSpeed{ 0.f };
		float m_ColorCycleSpeed{ 0.f };
		float m_Lifetime{ 0.f }; // 0 => persistent (no LifetimeComponent)
	};

	void CreateQuad(pg::CheckedRegistryAccessor& accessor, const QuadSpec& spec)
	{
		pg::ecs::Entity ent = accessor.Create();

		sbx::QuadComponent quad;
		quad.m_Color = spec.m_BaseColor;
		quad.m_TextureID = spec.m_TextureID;
		accessor.EmplaceDeferred<sbx::QuadComponent>(ent, std::move(quad));

		// Seed the quad's full transform once; QuadAnimationSystem updates position/rotation each frame.
		sbx::QuadSpawnTransformRequestOneFrameComponent transformRequest;
		transformRequest.m_Data.m_SetPosition = true;
		transformRequest.m_Data.m_Position = spec.m_Anchor;
		transformRequest.m_Data.m_SetRotation = true;
		transformRequest.m_Data.m_Rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
		transformRequest.m_Data.m_SetScale = true;
		transformRequest.m_Data.m_Scale = spec.m_Scale;
		transformRequest.m_Data.m_SetBounds = true;
		transformRequest.m_Data.m_BoundsMin = glm::vec3(0.f, 0.f, 0.f);
		transformRequest.m_Data.m_BoundsMax = glm::vec3(1.f, 1.f, 0.f);
		accessor.EmplaceOneframe<sbx::QuadSpawnTransformRequestOneFrameComponent>(ent, std::move(transformRequest));

		sbx::SpinComponent spin;
		spin.m_Anchor = spec.m_Anchor;
		spin.m_Scale = spec.m_Scale;
		spin.m_BaseColor = spec.m_BaseColor;
		spin.m_RotationSpeed = spec.m_RotationSpeed;
		spin.m_OrbitRadius = spec.m_OrbitRadius;
		spin.m_OrbitSpeed = spec.m_OrbitSpeed;
		spin.m_ColorCycleSpeed = spec.m_ColorCycleSpeed;
		accessor.EmplaceDeferred<sbx::SpinComponent>(ent, std::move(spin));

		if (spec.m_Lifetime > 0.f)
		{
			sbx::LifetimeComponent lifetime;
			lifetime.m_Remaining = spec.m_Lifetime;
			accessor.EmplaceDeferred<sbx::LifetimeComponent>(ent, std::move(lifetime));
		}
	}

	void SeedScene(pg::CheckedRegistryAccessor& accessor, const sbx::SandboxConfigSingletonComponent& config)
	{
		// Flat colour quad, slowly rotating, left of centre.
		QuadSpec flat;
		flat.m_Anchor = { -1.4f, 0.f, 0.f };
		flat.m_Scale = { 0.5f, 0.5f, 1.f };
		flat.m_BaseColor = { 0.2f, 0.85f, 0.35f };
		flat.m_RotationSpeed = 1.0f;
		CreateQuad(accessor, flat);

		// Textured quad at centre.
		QuadSpec textured;
		textured.m_Anchor = { 0.f, 0.f, 0.f };
		textured.m_Scale = { 0.6f, 0.6f, 1.f };
		textured.m_TextureID = config.m_TexturedQuadTextureID;
		textured.m_RotationSpeed = 0.4f;
		CreateQuad(accessor, textured);

		// Orbiting, colour-cycling quad.
		QuadSpec orbiter;
		orbiter.m_Anchor = { 0.f, 0.f, 0.f };
		orbiter.m_Scale = { 0.2f, 0.2f, 1.f };
		orbiter.m_OrbitRadius = 1.1f;
		orbiter.m_OrbitSpeed = 1.5f;
		orbiter.m_ColorCycleSpeed = 1.0f;
		CreateQuad(accessor, orbiter);

		// Two overlapping quads; the lower one (smaller Y) draws behind the higher one.
		QuadSpec behindQuad;
		behindQuad.m_Anchor = { 1.3f, -0.1f, 0.f };
		behindQuad.m_Scale = { 0.5f, 0.5f, 1.f };
		behindQuad.m_BaseColor = { 0.9f, 0.25f, 0.25f };
		CreateQuad(accessor, behindQuad);

		QuadSpec frontQuad;
		frontQuad.m_Anchor = { 1.5f, 0.1f, 0.f };
		frontQuad.m_Scale = { 0.5f, 0.5f, 1.f };
		frontQuad.m_BaseColor = { 0.25f, 0.4f, 0.95f };
		CreateQuad(accessor, frontQuad);
	}

	bool WasKeyJustPressed(const pg::InputStateSingletonComponent& input, int key)
	{
		const std::unordered_map<int, int>::const_iterator it = input.m_KeysPressed.find(key);
		return it != input.m_KeysPressed.end() && it->second == 1;
	}
}

pg::SystemAccessDecl sbx::QuadSpawnSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(sbx::SandboxConfigSingletonComponent)),
		std::type_index(typeid(pg::InputStateSingletonComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(sbx::SpawnerSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(sbx::QuadComponent)),
		std::type_index(typeid(sbx::QuadSpawnTransformRequestOneFrameComponent)),
		std::type_index(typeid(sbx::SpinComponent)),
		std::type_index(typeid(sbx::LifetimeComponent)),
		std::type_index(typeid(sbx::SpawnerSingletonComponent)),
	};
	return decl;
}

void sbx::QuadSpawnSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto configView = accessor.View<const sbx::SandboxConfigSingletonComponent>();
	if (configView.empty())
	{
		return;
	}
	const sbx::SandboxConfigSingletonComponent& config = configView.get<const sbx::SandboxConfigSingletonComponent>(configView.front());

	auto spawnerView = accessor.View<sbx::SpawnerSingletonComponent>();
	if (spawnerView.empty())
	{
		SeedScene(accessor, config);
		pg::ecs::Entity spawnerEnt = accessor.Create();
		accessor.EmplaceDeferred<sbx::SpawnerSingletonComponent>(spawnerEnt);
		return;
	}

	auto inputView = accessor.View<const pg::InputStateSingletonComponent>();
	if (inputView.empty())
	{
		return;
	}
	const pg::InputStateSingletonComponent& input = inputView.get<const pg::InputStateSingletonComponent>(inputView.front());

	if (WasKeyJustPressed(input, pg::PG_KEY_SPACE))
	{
		QuadSpec spawned;
		spawned.m_Anchor = { 0.f, -0.6f, 0.f };
		spawned.m_Scale = { 0.15f, 0.15f, 1.f };
		spawned.m_RotationSpeed = 3.0f;
		spawned.m_ColorCycleSpeed = 2.0f;
		spawned.m_Lifetime = 3.0f;
		CreateQuad(accessor, spawned);

		sbx::SpawnerSingletonComponent& spawner = spawnerView.get<sbx::SpawnerSingletonComponent>(spawnerView.front());
		++spawner.m_SpawnCount;
	}
}
