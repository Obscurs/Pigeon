#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include <Pigeon/ECS/World.h>
#include <Pigeon/ECS/System.h>
#include <Pigeon/Core/EngineConfigSingletonComponent.h>
#include <Pigeon/Renderer/OrthographicCameraComponent.h>
#include <Pigeon/Core/ResourceMapSingletonComponent.h>
#include <Pigeon/Renderer/Renderer2DSystem.h>
#include <Pigeon/Renderer/RendererDataSingletonComponent.h>
#include <Pigeon/Renderer/DrawQuadInFrameEvent.h>
#include <Pigeon/Renderer/DrawSpriteInFrameEvent.h>
#include <Pigeon/Renderer/DrawStringInFrameEvent.h>
#include <Pigeon/Renderer/DrawUIQuadInFrameEvent.h>
#include <Pigeon/Renderer/DrawUIStringInFrameEvent.h>
#include <Pigeon/Renderer/Shader.h>
#include <Pigeon/Renderer/Texture.h>

#include <glm/glm.hpp>
#include <vector>

namespace
{
	// Seed the minimal valid state for the system to initialise renderer data:
	// a camera, a resource map (default texture + quad/text shaders) and an engine
	// config whose shader IDs match the resource map's shader keys.
	void SeedValidRenderState()
	{
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();

		const pg::UUID quadShaderID = pg::UUID::Generate();
		const pg::UUID textShaderID = pg::UUID::Generate();

		pg::ecs::Entity camEnt = registry.create();
		registry.emplace<pg::OrthographicCameraComponent>(camEnt);

		pg::ecs::Entity resEnt = registry.create();
		pg::ResourceMapSingletonComponent& resources = registry.emplace<pg::ResourceMapSingletonComponent>(resEnt);
		std::vector<unsigned char> pixels(2 * 2 * 4, 255);
		resources.m_TextureMap[resources.m_DefaultTexture] = pg::MappedTexture{ pg::Texture2D::Create(2, 2, 4, pixels.data()), pg::EMappedTextureType::eQuad };
		resources.m_ShaderMap[quadShaderID] = pg::Shader::Create("UTTestQuadShader");
		resources.m_ShaderMap[textShaderID] = pg::Shader::Create("UTTestTextShader");

		pg::ecs::Entity cfgEnt = registry.create();
		pg::EngineConfigSingletonComponent& config = registry.emplace<pg::EngineConfigSingletonComponent>(cfgEnt);
		config.m_DefaultQuadShaderID = quadShaderID;
		config.m_DefaultTextShaderID = textShaderID;
	}
} // namespace

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Guard: no camera -> system returns early, no crash, no renderer data created
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Renderer2DSystem::NoOpWithoutCamera")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::Renderer2DSystem>());

		// Provide resources and config but NOT a camera.
		pg::ecs::Entity resEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::ResourceMapSingletonComponent>(resEnt);

		pg::ecs::Entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::EngineConfigSingletonComponent>(cfgEnt);

		world.Update(pg::Timestep(0));

		// No RendererDataSingletonComponent should have been created.
		auto view = pg::World::GetRegistryDirect().view<pg::RendererDataSingletonComponent>();
		CHECK(view.size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Guard: no resources -> system returns early, no crash
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Renderer2DSystem::NoOpWithoutResourceMap")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::Renderer2DSystem>());

		// Provide camera and config but NOT resources.
		pg::ecs::Entity camEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::OrthographicCameraComponent>(camEnt);

		pg::ecs::Entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::EngineConfigSingletonComponent>(cfgEnt);

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::RendererDataSingletonComponent>();
		CHECK(view.size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Guard: no engine config -> system returns early, no crash
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Renderer2DSystem::NoOpWithoutEngineConfig")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::Renderer2DSystem>());

		// Provide camera and resources but NOT engine config.
		pg::ecs::Entity camEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::OrthographicCameraComponent>(camEnt);

		pg::ecs::Entity resEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::ResourceMapSingletonComponent>(resEnt);

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::RendererDataSingletonComponent>();
		CHECK(view.size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Happy path: camera + resources + engine config present and no renderer data
	// yet -> system creates and fully initialises a RendererDataSingletonComponent.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Renderer2DSystem::InitialisesRendererDataWhenStateValid")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::Renderer2DSystem>());

		SeedValidRenderState();

		// A queued draw request exercises the render path; batches are flushed
		// before the frame ends, so it leaves no observable batch state behind.
		pg::ecs::Entity drawEnt = pg::World::GetRegistryDirect().create();
		pg::DrawQuadInFrameEvent drawEvent;
		drawEvent.m_Transform = glm::mat4(1.f);
		pg::World::GetRegistryDirect().emplace<pg::DrawQuadInFrameEvent>(drawEnt, drawEvent);

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::RendererDataSingletonComponent>();
		REQUIRE(view.size() == 1);

		const pg::RendererDataSingletonComponent& data =
			view.get<pg::RendererDataSingletonComponent>(view.front());
		CHECK(data.m_VertexBuffer != nullptr);
		CHECK(data.m_IndexBuffer != nullptr);
		CHECK(data.m_QuadShader != nullptr);
		CHECK(data.m_TextShader != nullptr);
		// Batches are flushed and cleared by the end of the frame.
		CHECK(data.m_BatchMap.empty());
		CHECK(data.m_LayerBatchMap.empty());
	}

	// ---------------------------------------------------------------------------
	// Edge: renderer data already exists -> system reuses it (write path), it does
	// not create a duplicate singleton.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Renderer2DSystem::ReusesRendererDataOnSubsequentFrame")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::Renderer2DSystem>());

		SeedValidRenderState();

		world.Update(pg::Timestep(0));
		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::RendererDataSingletonComponent>();
		CHECK(view.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Renderer2DSystem::DeclareAccessIsCorrect")
	{
		pg::Renderer2DSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::EngineConfigSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::OrthographicCameraComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ResourceMapSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::DrawQuadInFrameEvent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::DrawSpriteInFrameEvent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::DrawStringInFrameEvent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::DrawUIQuadInFrameEvent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::DrawUIStringInFrameEvent))) > 0);

		CHECK(decl.addSet.count(std::type_index(typeid(pg::RendererDataSingletonComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(pg::RendererDataSingletonComponent))) > 0);
	}

} // namespace CatchTestsetFail
