#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Core/EngineConfigSingletonComponent.h"
#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/Model.h"
#include "Pigeon/Renderer/ModelComponent.h"
#include "Pigeon/Renderer/PerspectiveCameraComponent.h"
#include "Pigeon/Renderer/RenderTarget.h"
#include "Pigeon/Renderer/Renderer3DDataSingletonComponent.h"
#include "Pigeon/Renderer/Renderer3DSystem.h"
#include "Pigeon/Renderer/Shader.h"
#include "Pigeon/Transform/WorldTransformComponent.h"
#include "Platform/Testing/TestingHelper.h"

#include <glm/glm.hpp>

namespace
{
	const pg::UUID s_ShaderID = pg::UUID("c3000000-0000-4000-8000-000000000001");
	const pg::UUID s_RenderTargetID = pg::UUID("c3000000-0000-4000-8000-000000000002");
	const pg::UUID s_ModelID = pg::UUID("c3000000-0000-4000-8000-0000000000aa");

	// Seeds the minimal valid state for the 3D pass: an engine config naming the 3D shader + render
	// target, a resource map holding both plus a loaded model, and a perspective camera.
	void SeedValid3DState()
	{
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();

		pg::ecs::Entity cfgEnt = registry.create();
		pg::EngineConfigSingletonComponent& config = registry.emplace<pg::EngineConfigSingletonComponent>(cfgEnt);
		config.m_Model3DShaderID = s_ShaderID;
		config.m_Render3DTargetID = s_RenderTargetID;

		pg::ecs::Entity resEnt = registry.create();
		pg::ResourceMapSingletonComponent& resources = registry.emplace<pg::ResourceMapSingletonComponent>(resEnt);
		resources.m_ShaderMap[s_ShaderID] = pg::Shader::Create("UTTest3DShader");
		pg::S_Ptr<pg::RenderTarget> renderTarget = pg::RenderTarget::Create(64, 64);
		resources.m_RenderTargetMap[s_RenderTargetID] = renderTarget;
		resources.m_TextureMap[s_RenderTargetID] = pg::MappedTexture{ renderTarget->GetColorTexture(), pg::EMappedTextureType::eQuad };
		resources.m_ModelMap[s_ModelID] = pg::Model::Create("Assets/UT/Models/quad.obj");

		pg::ecs::Entity camEnt = registry.create();
		registry.emplace<pg::PerspectiveCameraComponent>(camEnt);
	}

	// Creates a 3D entity referencing the seeded model with an identity world transform.
	void CreateModelEntity()
	{
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		pg::ecs::Entity ent = registry.create();
		pg::ModelComponent& model = registry.emplace<pg::ModelComponent>(ent);
		model.m_ModelID = s_ModelID;
		registry.emplace<pg::WorldTransformComponent>(ent);
	}
}

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Guard: no perspective camera -> system returns early, no renderer data created.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Renderer3DSystem::NoOpWithoutCamera")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::Renderer3DSystem>());

		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		pg::ecs::Entity cfgEnt = registry.create();
		pg::EngineConfigSingletonComponent& config = registry.emplace<pg::EngineConfigSingletonComponent>(cfgEnt);
		config.m_Model3DShaderID = s_ShaderID;
		config.m_Render3DTargetID = s_RenderTargetID;

		pg::ecs::Entity resEnt = registry.create();
		pg::ResourceMapSingletonComponent& resources = registry.emplace<pg::ResourceMapSingletonComponent>(resEnt);
		resources.m_ShaderMap[s_ShaderID] = pg::Shader::Create("UTTest3DShader");
		resources.m_RenderTargetMap[s_RenderTargetID] = pg::RenderTarget::Create(64, 64);

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::Renderer3DDataSingletonComponent>();
		CHECK(view.size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Guard: configured render target absent from the resource map -> no-op.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Renderer3DSystem::NoOpWithoutRenderTarget")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::Renderer3DSystem>());

		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		pg::ecs::Entity cfgEnt = registry.create();
		pg::EngineConfigSingletonComponent& config = registry.emplace<pg::EngineConfigSingletonComponent>(cfgEnt);
		config.m_Model3DShaderID = s_ShaderID;
		config.m_Render3DTargetID = s_RenderTargetID;

		pg::ecs::Entity resEnt = registry.create();
		pg::ResourceMapSingletonComponent& resources = registry.emplace<pg::ResourceMapSingletonComponent>(resEnt);
		resources.m_ShaderMap[s_ShaderID] = pg::Shader::Create("UTTest3DShader");

		pg::ecs::Entity camEnt = registry.create();
		registry.emplace<pg::PerspectiveCameraComponent>(camEnt);

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::Renderer3DDataSingletonComponent>();
		CHECK(view.size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Happy path: valid state + a model entity -> the system creates and fully
	// initialises the renderer-data singleton (buffers + shader).
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Renderer3DSystem::InitialisesRendererDataWhenStateValid")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::Renderer3DSystem>());

		SeedValid3DState();
		CreateModelEntity();

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::Renderer3DDataSingletonComponent>();
		REQUIRE(view.size() == 1);

		const pg::Renderer3DDataSingletonComponent& data =
			view.get<pg::Renderer3DDataSingletonComponent>(view.front());
		CHECK(data.m_VertexBuffer != nullptr);
		CHECK(data.m_IndexBuffer != nullptr);
		CHECK(data.m_ModelShader != nullptr);
	}

	// ---------------------------------------------------------------------------
	// Happy path: the model's geometry is uploaded to the GPU buffers. The test
	// quad is four vertices and six indices.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Renderer3DSystem::UploadsModelGeometry")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::Renderer3DSystem>());

		SeedValid3DState();
		CreateModelEntity();

		pg::TestingHelper::Reset();
		world.Update(pg::Timestep(0));

		const std::vector<pg::TestingHelper::VertexBufferSetVerticesData>& vertexUploads =
			pg::TestingHelper::GetInstance().m_VertexBufferSetVertices;
		const std::vector<pg::TestingHelper::IndexBufferSetVerticesData>& indexUploads =
			pg::TestingHelper::GetInstance().m_IndexBufferSetIndices;

		REQUIRE(vertexUploads.size() == 1);
		CHECK(vertexUploads[0].m_Count == 4);
		REQUIRE(indexUploads.size() == 1);
		CHECK(indexUploads[0].m_Count == 6);
	}

	// ---------------------------------------------------------------------------
	// Edge: renderer data already exists -> the system reuses it (write path), it
	// does not create a duplicate singleton.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Renderer3DSystem::ReusesRendererDataOnSubsequentFrame")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::Renderer3DSystem>());

		SeedValid3DState();
		CreateModelEntity();

		world.Update(pg::Timestep(0));
		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::Renderer3DDataSingletonComponent>();
		CHECK(view.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Renderer3DSystem::DeclareAccessIsCorrect")
	{
		pg::Renderer3DSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::EngineConfigSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ResourceMapSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::PerspectiveCameraComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ModelComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::WorldTransformComponent))) > 0);

		CHECK(decl.writeSet.count(std::type_index(typeid(pg::Renderer3DDataSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::Renderer3DDataSingletonComponent))) > 0);
	}
}
