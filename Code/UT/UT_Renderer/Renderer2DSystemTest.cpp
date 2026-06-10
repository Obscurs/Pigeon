#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Core/EngineConfigSingletonComponent.h"
#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/DrawQuadInFrameEvent.h"
#include "Pigeon/Renderer/DrawSpriteInFrameEvent.h"
#include "Pigeon/Renderer/DrawStringInFrameEvent.h"
#include "Pigeon/Renderer/DrawUIQuadInFrameEvent.h"
#include "Pigeon/Renderer/DrawUIStringInFrameEvent.h"
#include "Pigeon/Renderer/OrthographicCameraComponent.h"
#include "Pigeon/Renderer/Renderer2DSystem.h"
#include "Pigeon/Renderer/RendererDataSingletonComponent.h"
#include "Pigeon/Renderer/Shader.h"
#include "Pigeon/Renderer/UICameraSingletonComponent.h"
#include "Pigeon/Renderer/Texture.h"
#include "Platform/Testing/TestingHelper.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
		CHECK(decl.readSet.count(std::type_index(typeid(pg::UICameraSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ResourceMapSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::DrawQuadInFrameEvent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::DrawSpriteInFrameEvent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::DrawStringInFrameEvent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::DrawUIQuadInFrameEvent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::DrawUIStringInFrameEvent))) > 0);

		CHECK(decl.addSet.count(std::type_index(typeid(pg::RendererDataSingletonComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(pg::RendererDataSingletonComponent))) > 0);
	}

	// ---------------------------------------------------------------------------
	// World draws are ordered by sort key: lower sort key is written (drawn) first.
	// Two quads sharing the default texture batch into a single submission, so the
	// first vertices in the buffer belong to the lower-sort-key quad.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Renderer2DSystem::OrdersWorldDrawsBySortKey")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::Renderer2DSystem>());
		SeedValidRenderState();

		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();

		// Quad A: high sort key, placed at x = 5. Emitted first.
		pg::ecs::Entity entA = registry.create();
		pg::DrawQuadInFrameEvent a;
		a.m_Transform = glm::translate(glm::mat4(1.f), glm::vec3(5.f, 0.f, 0.f));
		a.m_SortKey = 5.f;
		registry.emplace<pg::DrawQuadInFrameEvent>(entA, a);

		// Quad B: low sort key, placed at x = 1. Emitted second but must draw first.
		pg::ecs::Entity entB = registry.create();
		pg::DrawQuadInFrameEvent b;
		b.m_Transform = glm::translate(glm::mat4(1.f), glm::vec3(1.f, 0.f, 0.f));
		b.m_SortKey = 1.f;
		registry.emplace<pg::DrawQuadInFrameEvent>(entB, b);

		world.Update(pg::Timestep(0));

		// Same texture -> a single batched submission of two quads (8 vertices).
		REQUIRE(pg::TestingHelper::GetInstance().m_VertexBufferSetVertices.size() == 1);
		CHECK(pg::TestingHelper::GetInstance().m_VertexBufferSetVertices[0].m_Count == 8);
		// First vertex written belongs to the lower-sort-key quad (x = 1).
		CHECK(pg::TestingHelper::GetInstance().m_Vertices[pg::ATRIB_POS_X_INDEX] == Approx(1.f));
	}

	// ---------------------------------------------------------------------------
	// UI pass applies the draw event's clip rect as a window-pixel scissor.
	// A UI quad carrying a clip rect, drawn through the UI camera, records that scissor.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Renderer2DSystem::UIPassAppliesClipScissor")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::Renderer2DSystem>());
		SeedValidRenderState();

		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();

		// The UI camera is added in production by UIRenderSystem; supplied here so the UI pass runs.
		pg::ecs::Entity uiCamEnt = registry.create();
		registry.emplace<pg::UICameraSingletonComponent>(uiCamEnt);

		// A UI quad (default texture) masked to a clip rect.
		pg::ecs::Entity uiQuadEnt = registry.create();
		pg::DrawUIQuadInFrameEvent uiQuad;
		uiQuad.m_Transform = glm::mat4(1.f);
		uiQuad.m_ClipRect = glm::vec4(10.f, 20.f, 100.f, 50.f);
		registry.emplace<pg::DrawUIQuadInFrameEvent>(uiQuadEnt, uiQuad);

		world.Update(pg::Timestep(0));

		const std::vector<pg::TestingHelper::ScissorData>& scissors = pg::TestingHelper::GetInstance().m_Scissors;
		bool found = false;
		for (const pg::TestingHelper::ScissorData& s : scissors)
		{
			if (s.m_X == 10 && s.m_Y == 20 && s.m_Width == 100 && s.m_Height == 50)
			{
				found = true;
			}
		}
		CHECK(found);
	}

	// ---------------------------------------------------------------------------
	// UI quad samples its event's UV sub-rect (nine-slice cells rely on this): the
	// first emitted vertex carries the sub-rect's (u0, v0).
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Renderer2DSystem::UIQuadUsesEventTexCoords")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::Renderer2DSystem>());
		SeedValidRenderState();

		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();

		pg::ecs::Entity uiCamEnt = registry.create();
		registry.emplace<pg::UICameraSingletonComponent>(uiCamEnt);

		pg::ecs::Entity uiQuadEnt = registry.create();
		pg::DrawUIQuadInFrameEvent uiQuad;
		uiQuad.m_Transform = glm::mat4(1.f);
		uiQuad.m_TexCoords = glm::vec4(0.25f, 0.5f, 0.75f, 1.0f);
		registry.emplace<pg::DrawUIQuadInFrameEvent>(uiQuadEnt, uiQuad);

		world.Update(pg::Timestep(0));

		// The first vertex samples the sub-rect's minimum UV corner.
		CHECK(pg::TestingHelper::GetInstance().m_Vertices[pg::ATRIB_TEX_X_INDEX] == Approx(0.25f));
		CHECK(pg::TestingHelper::GetInstance().m_Vertices[pg::ATRIB_TEX_Y_INDEX] == Approx(0.5f));
	}

	// ---------------------------------------------------------------------------
	// Draws with different textures cannot share a batch: a texture change forces a
	// new submission, so two differently-textured quads produce two submissions.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Renderer2DSystem::SeparatesBatchesByTexture")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::Renderer2DSystem>());
		SeedValidRenderState();

		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		auto resourcesView = registry.view<pg::ResourceMapSingletonComponent>();
		pg::ResourceMapSingletonComponent& resources = resourcesView.get<pg::ResourceMapSingletonComponent>(resourcesView.front());
		const pg::UUID secondTexture = pg::UUID::Generate();
		std::vector<unsigned char> pixels(2 * 2 * 4, 255);
		resources.m_TextureMap[secondTexture] = pg::MappedTexture{ pg::Texture2D::Create(2, 2, 4, pixels.data()), pg::EMappedTextureType::eQuad };

		// Lower sort key uses the default texture; higher uses the second texture.
		pg::ecs::Entity e1 = registry.create();
		pg::DrawQuadInFrameEvent q1;
		q1.m_Transform = glm::mat4(1.f);
		q1.m_SortKey = 0.f;
		registry.emplace<pg::DrawQuadInFrameEvent>(e1, q1);

		pg::ecs::Entity e2 = registry.create();
		pg::DrawQuadInFrameEvent q2;
		q2.m_Transform = glm::mat4(1.f);
		q2.m_TextureID = secondTexture;
		q2.m_SortKey = 1.f;
		registry.emplace<pg::DrawQuadInFrameEvent>(e2, q2);

		world.Update(pg::Timestep(0));

		CHECK(pg::TestingHelper::GetInstance().m_VertexBufferSetVertices.size() == 2);
	}

} // namespace CatchTestsetFail
