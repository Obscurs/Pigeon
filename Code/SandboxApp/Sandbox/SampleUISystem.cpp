#include "SampleUISystem.h"

#include <imgui.h>

#include "Pigeon/Core/Clock.h"
#include "Pigeon/Core/OrthographicCameraComponent.h"
#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/DrawQuadInFrameEvent.h"
#include "Pigeon/Renderer/DrawSpriteInFrameEvent.h"
#include "Pigeon/Renderer/DrawStringInFrameEvent.h"
#include "Pigeon/Renderer/Sprite.h"
#include "Pigeon/UI/UIComponents.h"
#include "Sandbox/SampleUIConfigSingletonComponent.h"
#include "Sandbox/SampleUISingletonComponent.h"

pig::SystemAccessDecl sbx::SampleUISystem::DeclareAccess() const
{
	pig::SystemAccessDecl decl;
	decl.writeSet = {
		std::type_index(typeid(sbx::SampleUISingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pig::OrthographicCameraComponent)),
		std::type_index(typeid(pig::ui::LoadLayoutEvent)),
		std::type_index(typeid(sbx::SampleUISingletonComponent)),
	};
	decl.inframeAddSet = {

		std::type_index(typeid(pig::DrawQuadInFrameEvent)),
		std::type_index(typeid(pig::DrawSpriteInFrameEvent)),
		std::type_index(typeid(pig::DrawStringInFrameEvent)),
	};
	decl.readSet = {
		std::type_index(typeid(sbx::SampleUIConfigSingletonComponent)),
		std::type_index(typeid(pig::ResourceMapSingletonComponent)),
		std::type_index(typeid(pig::ui::BaseComponent)),
		std::type_index(typeid(pig::ui::ImageComponent)),
		std::type_index(typeid(pig::ui::TextComponent)),
	};
	return decl;
}

void sbx::SampleUISystem::Update(const pig::Timestep& ts)
{
	auto accessor = pig::World::GetRegistry();

	auto resourcesView = accessor.view<const pig::ResourceMapSingletonComponent>();
	auto configView = accessor.view<const sbx::SampleUIConfigSingletonComponent>();
	if (resourcesView.empty() || configView.empty())
	{
		return;
	}
	const pig::ResourceMapSingletonComponent& resourcesComponent = resourcesView.get<const pig::ResourceMapSingletonComponent>(resourcesView.front());
	const sbx::SampleUIConfigSingletonComponent& configComponent = configView.get<const sbx::SampleUIConfigSingletonComponent>(configView.front());
	auto view = accessor.view<sbx::SampleUISingletonComponent>();
	if (view.empty())
	{
		sbx::SampleUISingletonComponent component;
		entt::entity entity = accessor.create();

		entt::entity cameraEntity = accessor.create();

		pig::OrthographicCameraComponent cameraComponent;

		cameraComponent.m_AspectRatio = 1280.0f / 720.0f;
		cameraComponent.m_Camera = pig::OrthographicCamera(-cameraComponent.m_AspectRatio * cameraComponent.m_ZoomLevel, cameraComponent.m_AspectRatio * cameraComponent.m_ZoomLevel, -cameraComponent.m_ZoomLevel, cameraComponent.m_ZoomLevel);
		cameraComponent.m_Rotation = false;
		cameraComponent.m_ReactsToInput = true;
		cameraComponent.m_CameraPosition.z = 0.f;

		component.m_ColorQuad1 = glm::vec3(0.f, 1.f, 0.0f);
		component.m_PosQuad1 = glm::vec3(0.0f, 0.0f, 0.0f);
		component.m_ScaleQuad1 = glm::vec3(0.1f, 0.1f, 1.f);
		component.m_OriginQuad1 = glm::vec3(0.5f, 0.5f, 0.f);

		component.m_ColorQuad2 = glm::vec3(0.f, 1.f, 1.f);
		component.m_PosQuad2 = glm::vec3(1.f, 1.f, 0.f);
		component.m_ScaleQuad2 = glm::vec3(0.5f, 1.0f, 1.f);
		component.m_OriginQuad2 = glm::vec3(0.5f, 0.5f, 0.f);

		component.m_PosText = glm::vec3(0.f, 0.f, 0.f);
		component.m_ScaleText = glm::vec3(0.3f, 0.3f, 1.f);
		component.m_ColorText = glm::vec3(1.f, 0.f, 0.f);

		component.m_FontID = pig::UUID(configComponent.m_DefaultFontID);
		component.m_UUIDUI1 = pig::UUID(configComponent.m_UUIDUI1);
		component.m_UUIDUI2 = pig::UUID(configComponent.m_UUIDUI2);

		accessor.emplace_deferred<sbx::SampleUISingletonComponent>(entity, std::move(component));

		pig::ui::LoadLayoutEvent layoutEvent;
		layoutEvent.m_UUID = configComponent.m_MainLayoutID;
		accessor.EmplaceEvent<pig::ui::LoadLayoutEvent>(std::move(layoutEvent));

		accessor.emplace_deferred<pig::OrthographicCameraComponent>(cameraEntity, std::move(cameraComponent));
	}
	else
	{
		for (auto ent : view)
		{
			sbx::SampleUISingletonComponent& component = view.get<sbx::SampleUISingletonComponent>(ent);

			glm::mat4 transformQuad1(1.f);
			transformQuad1 = glm::translate(transformQuad1, component.m_PosQuad1);
			transformQuad1 = glm::scale(transformQuad1, component.m_ScaleQuad1);

			glm::mat4 transformGrid(1.f);
			transformGrid = glm::translate(transformGrid, glm::vec3(-4.f, -4.f, 0.f));
			transformGrid = glm::scale(transformGrid, glm::vec3(8.f, 8.f, 0.f));

			pig::DrawQuadInFrameEvent quadEvent;
			quadEvent.m_Transform = transformQuad1;
			quadEvent.m_Origin = glm::vec3(0.f, 0.f, 0.f);
			quadEvent.m_Color = component.m_ColorQuad1;
			accessor.EmplaceInframeEvent<pig::DrawQuadInFrameEvent>(std::move(quadEvent));

			auto viewImageUI = accessor.view<const pig::ui::BaseComponent, const pig::ui::ImageComponent>();
			for (auto ent : viewImageUI)
			{
				const pig::ui::BaseComponent& baseComponent = viewImageUI.get<const pig::ui::BaseComponent>(ent);
				const pig::ui::ImageComponent& imageComponent = viewImageUI.get<const pig::ui::ImageComponent>(ent);
				if (baseComponent.m_UUID == pig::UUID(configComponent.m_UUIDUI1))
				{
					const glm::vec4 texCoordsRect(32, 32, 64, 64);

					glm::mat4 transformQuad2(1.f);
					transformQuad2 = glm::translate(transformQuad2, component.m_PosQuad2);
					transformQuad2 = glm::scale(transformQuad2, component.m_ScaleQuad2);
					pig::Sprite sprite(transformQuad2, texCoordsRect, imageComponent.m_TextureHandle, component.m_OriginQuad2);
					pig::DrawSpriteInFrameEvent spriteEvent(sprite);
					accessor.EmplaceInframeEvent<pig::DrawSpriteInFrameEvent>(std::move(spriteEvent));
				}
			}

			std::string textString("This is a fucking text\nEven with multiple lines");

			glm::mat4 stringTransform = glm::mat4(1.0f); // Identity matrix
			stringTransform = glm::translate(stringTransform, component.m_PosText); // Apply translation
			stringTransform = glm::scale(stringTransform, component.m_ScaleText); // Apply scaling

			pig::DrawStringInFrameEvent stringEvent;
			stringEvent.m_Transform = stringTransform;
			stringEvent.m_String = textString;
			PG_CORE_EXCEPT(resourcesComponent.m_FontMap.find(component.m_FontID) != resourcesComponent.m_FontMap.end(), "Could not text font");
			stringEvent.m_Font = resourcesComponent.m_FontMap.at(component.m_FontID);
			stringEvent.m_Color = glm::vec4(component.m_ColorText, 1.f);
			stringEvent.m_Kerning = 0.1f;
			stringEvent.m_Linespacing = 0.1f;
			accessor.EmplaceInframeEvent<pig::DrawStringInFrameEvent>(std::move(stringEvent));

			static pig::Clock clock;
			const pig::Timestep elapsed{ clock.Elapsed() };
			const std::string timeString
			{
				std::to_string(static_cast<int>(elapsed.AsMinutes())) + ":" +
				std::to_string(static_cast<int>(elapsed.AsSeconds()) % 60) + ":" +
				std::to_string(elapsed.AsMilliseconds() % 1000)
			};
			const glm::vec3 timeTextColor
			{
				std::fmodf(elapsed.AsSeconds() * 8.f, 1.f),
				std::fmodf(elapsed.AsSeconds() * 2.f, 1.f),
				std::fmodf(elapsed.AsSeconds() * 4.f, 1.f),
			};

			glm::mat4 stringClockTransform = glm::mat4(1.0f); // Identity matrix
			stringClockTransform = glm::translate(stringClockTransform, glm::vec3(-0.7f, 0.7f, 0.f)); // Apply translation
			stringClockTransform = glm::scale(stringClockTransform, glm::vec3(0.3f, 0.3f, 1.0f)); // Apply scaling

			ImGui::Begin("Settings");
			ImGui::ColorEdit3("Quad1 Color", glm::value_ptr(component.m_ColorQuad1));
			ImGui::InputFloat3("Quad1 Position", glm::value_ptr(component.m_PosQuad1));
			ImGui::InputFloat3("Quad1 Scale", glm::value_ptr(component.m_ScaleQuad1));
			ImGui::InputFloat3("Quad1 Origin", glm::value_ptr(component.m_OriginQuad1));

			ImGui::InputFloat3("Text Position", glm::value_ptr(component.m_PosText));
			ImGui::InputFloat3("Text Scale", glm::value_ptr(component.m_ScaleText));

			ImGui::End();
		}
	}
	
}