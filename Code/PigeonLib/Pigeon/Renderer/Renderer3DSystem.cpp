#include "pch.h"
#include "Pigeon/Renderer/Renderer3DSystem.h"

#include "Pigeon/Core/EngineConfigSingletonComponent.h"
#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/Model.h"
#include "Pigeon/Renderer/ModelComponent.h"
#include "Pigeon/Renderer/PerspectiveCameraComponent.h"
#include "Pigeon/Renderer/RenderCommand.h"
#include "Pigeon/Renderer/RenderTarget.h"
#include "Pigeon/Renderer/Renderer3DDataSingletonComponent.h"
#include "Pigeon/Transform/WorldTransformComponent.h"

#include <glm/glm.hpp>
#include <utility>
#include <vector>

namespace
{
	// Dark offscreen background so the rendered model reads clearly when sampled onto a 2D quad.
	static const glm::vec4 s_ClearColor{ 0.10f, 0.12f, 0.18f, 1.f };

	static const unsigned int MODEL_VERTEX_FLOATS = sizeof(pg::ModelVertex) / sizeof(float);

	void Init(pg::Renderer3DDataSingletonComponent& rendererData, const pg::ResourceMapSingletonComponent& resources, const pg::EngineConfigSingletonComponent& config)
	{
		const std::vector<float> emptyVertices(pg::MODEL_MAX_VERTICES * MODEL_VERTEX_FLOATS, 0.f);
		rendererData.m_VertexBuffer = pg::VertexBuffer::Create(emptyVertices.data(), pg::MODEL_MAX_VERTICES * sizeof(pg::ModelVertex), sizeof(pg::ModelVertex));

		const std::vector<uint32_t> emptyIndices(pg::MODEL_MAX_INDICES, 0);
		rendererData.m_IndexBuffer = pg::IndexBuffer::Create(emptyIndices.data(), pg::MODEL_MAX_INDICES);

		PG_CORE_EXCEPT(resources.m_ShaderMap.find(config.m_Model3DShaderID) != resources.m_ShaderMap.end(), "Could not find 3D model shader");
		rendererData.m_ModelShader = resources.m_ShaderMap.at(config.m_Model3DShaderID);
	}

	void Render(pg::CheckedRegistryAccessor& accessor, const pg::PerspectiveCamera& camera, pg::RenderTarget& renderTarget, pg::Renderer3DDataSingletonComponent& rendererData, const pg::ResourceMapSingletonComponent& resources)
	{
		pg::RenderCommand::BeginRenderTarget(renderTarget, s_ClearColor);

		rendererData.m_VertexBuffer->Bind();
		rendererData.m_IndexBuffer->Bind();
		rendererData.m_ModelShader->Bind();

		const glm::mat4 viewProjection = camera.GetViewProjectionMatrix();

		// Lambert light source sits at the camera position; constant per frame, so upload once.
		rendererData.m_ModelShader->UploadUniformFloat3("u_LightPos", camera.GetPosition());

		auto view = accessor.View<const pg::ModelComponent, const pg::WorldTransformComponent>();
		for (pg::ecs::Entity ent : view)
		{
			const pg::ModelComponent& modelComponent = view.get<const pg::ModelComponent>(ent);
			const pg::WorldTransformComponent& worldTransform = view.get<const pg::WorldTransformComponent>(ent);

			const std::unordered_map<pg::UUID, pg::S_Ptr<pg::Model>>::const_iterator it = resources.m_ModelMap.find(modelComponent.m_ModelID);
			if (it == resources.m_ModelMap.end() || it->second == nullptr)
			{
				continue;
			}

			const pg::Model& model = *it->second;
			// Skip empty models, and models too large for the fixed GPU buffers (uploading more would
			// overrun them).
			if (model.GetVertices().empty() || model.GetIndices().empty()
				|| model.GetVertices().size() > pg::MODEL_MAX_VERTICES || model.GetIndices().size() > pg::MODEL_MAX_INDICES)
			{
				continue;
			}

			// The shader applies a single combined matrix (the 2D multiply convention), so fold the world
			// transform into the view-projection here and upload it per model.
			const glm::mat4 modelViewProjection = viewProjection * worldTransform.m_Matrix;
			rendererData.m_ModelShader->UploadUniformMat4("u_ViewProjection", modelViewProjection);
			// The world matrix on its own, for transforming positions/normals into world space for lighting.
			rendererData.m_ModelShader->UploadUniformMat4("u_Transform", worldTransform.m_Matrix);

			rendererData.m_VertexBuffer->SetVertices(reinterpret_cast<const float*>(model.GetVertices().data()), static_cast<unsigned int>(model.GetVertices().size()), 0);
			rendererData.m_IndexBuffer->SetIndices(model.GetIndices().data(), static_cast<unsigned int>(model.GetIndices().size()), 0);

			pg::RenderCommand::DrawIndexed(static_cast<unsigned int>(model.GetIndices().size()));
		}

		pg::RenderCommand::EndRenderTarget();
	}
}

pg::SystemAccessDecl pg::Renderer3DSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::EngineConfigSingletonComponent)),
		std::type_index(typeid(pg::ResourceMapSingletonComponent)),
		std::type_index(typeid(pg::PerspectiveCameraComponent)),
		std::type_index(typeid(pg::ModelComponent)),
		std::type_index(typeid(pg::WorldTransformComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(pg::Renderer3DDataSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::Renderer3DDataSingletonComponent)),
	};
	return decl;
}

void pg::Renderer3DSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto configView = accessor.View<const pg::EngineConfigSingletonComponent>();
	auto resourcesView = accessor.View<const pg::ResourceMapSingletonComponent>();
	auto cameraView = accessor.View<const pg::PerspectiveCameraComponent>();
	if (configView.empty() || resourcesView.empty() || cameraView.empty())
	{
		return;
	}

	const pg::EngineConfigSingletonComponent& config = configView.get<const pg::EngineConfigSingletonComponent>(configView.front());
	const pg::ResourceMapSingletonComponent& resources = resourcesView.get<const pg::ResourceMapSingletonComponent>(resourcesView.front());
	const pg::PerspectiveCamera& camera = cameraView.get<const pg::PerspectiveCameraComponent>(cameraView.front()).m_Camera;

	// The render target + shader are loaded by ResourceManagerSystem; no-op until both are present.
	const std::unordered_map<pg::UUID, pg::S_Ptr<pg::RenderTarget>>::const_iterator renderTargetIt = resources.m_RenderTargetMap.find(config.m_Render3DTargetID);
	if (renderTargetIt == resources.m_RenderTargetMap.end() || renderTargetIt->second == nullptr)
	{
		return;
	}
	if (resources.m_ShaderMap.find(config.m_Model3DShaderID) == resources.m_ShaderMap.end())
	{
		return;
	}
	pg::RenderTarget& renderTarget = *renderTargetIt->second;

	auto rendererDataView = accessor.View<pg::Renderer3DDataSingletonComponent>();
	if (rendererDataView.empty())
	{
		pg::Renderer3DDataSingletonComponent rendererData;
		pg::ecs::Entity singletonEntity = accessor.Create();
		Init(rendererData, resources, config);
		Render(accessor, camera, renderTarget, rendererData, resources);
		accessor.EmplaceDeferred<pg::Renderer3DDataSingletonComponent>(singletonEntity, std::move(rendererData));
	}
	else
	{
		pg::Renderer3DDataSingletonComponent& rendererData = rendererDataView.get<pg::Renderer3DDataSingletonComponent>(rendererDataView.front());
		Render(accessor, camera, renderTarget, rendererData, resources);
	}
}
