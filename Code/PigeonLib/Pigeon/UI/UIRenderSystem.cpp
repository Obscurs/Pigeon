#include "Pigeon/UI/UIRenderSystem.h"

#include "Pigeon/Core/EngineConfigSingletonComponent.h"
#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/DrawUIQuadInFrameEvent.h"
#include "Pigeon/Renderer/DrawUIStringInFrameEvent.h"
#include "Pigeon/Renderer/Font.h"
#include "Pigeon/UI/UIComponents.h"
#include "Pigeon/UI/UIHelpers.h"

namespace
{
	glm::mat4 GetUIElementTransform(pg::CheckedRegistryAccessor& accessor, const pg::ui::BaseComponent& baseComponent, const pg::ui::RendererConfigSingletonComponent& renderComponent, const glm::vec2& uiTransformScale, const glm::vec2& uiBoundsSize)
	{
		int level = 0;
		const glm::vec4 bounds = pg::ui::GetGlobalBoundsForElement(accessor, baseComponent, renderComponent, uiBoundsSize, level);

		glm::mat4 transform(1.f);
		transform = glm::translate(transform, glm::vec3(bounds.x, bounds.y, level));
		transform = glm::scale(transform, glm::vec3(uiTransformScale, 1.f));
		return transform;
	}

	float GetFontSizeFromStringBounds(const pg::ui::BaseComponent& baseComponent, const glm::vec2 stringBounds, unsigned int numLines)
	{
		const float aspectUIBox = baseComponent.m_Size.x / baseComponent.m_Size.y;
		const float aspectStringBounds = stringBounds.x / stringBounds.y;

		float fontSize = 0;
		if (aspectUIBox > aspectStringBounds)
		{

			fontSize = baseComponent.m_Size.y / (stringBounds.y * numLines);
		}
		else
		{
			fontSize = baseComponent.m_Size.x / stringBounds.x;
		}

		return fontSize;
	}

	glm::vec2 GetStringBounds(const std::string& string, float kerning, float linespace, pg::S_Ptr<pg::Font> font)
	{
		return font->GetStringBounds(string, kerning, linespace);
	}

	unsigned int GetStringNumLines(const std::string& string, pg::S_Ptr<pg::Font> font)
	{
		if (string.empty())
			return 0;

		unsigned int numLines = 1;
		for (size_t i = 0; i < string.size(); i++)
		{
			if (font->IsCharacterNewLine(string[i]))
				++numLines;
		}
		return numLines;
	}
}

pg::SystemAccessDecl pg::ui::UIRenderSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::EngineConfigSingletonComponent)),
		std::type_index(typeid(pg::ResourceMapSingletonComponent)),
		std::type_index(typeid(pg::ui::BaseComponent)),
		std::type_index(typeid(pg::ui::ImageComponent)),
		std::type_index(typeid(pg::ui::TextComponent)),
		std::type_index(typeid(pg::ui::RendererConfigSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::ui::RendererConfigSingletonComponent)),
	};
	decl.inframeAddSet = {
		std::type_index(typeid(pg::DrawUIQuadInFrameEvent)),
		std::type_index(typeid(pg::DrawUIStringInFrameEvent)),
	};
	return decl;
}

void pg::ui::UIRenderSystem::Update(const pg::Timestep& ts)
{
	auto accessor = pg::World::GetRegistry();

	auto viewRenderConfig = accessor.View<const pg::ui::RendererConfigSingletonComponent>();
	if (viewRenderConfig.size() == 0)
	{
		pg::ui::RendererConfigSingletonComponent config;
		pg::ecs::Entity configEntity = accessor.Create();
		accessor.EmplaceDeferred<pg::ui::RendererConfigSingletonComponent>(configEntity, std::move(config));
		return;
	}
	PG_CORE_ASSERT(viewRenderConfig.size() == 1, "There should only be one ui render config component");

	auto viewEngineConfig = accessor.View<const pg::EngineConfigSingletonComponent>();
	auto viewResources = accessor.View<const pg::ResourceMapSingletonComponent>();
	if (viewEngineConfig.empty() || viewResources.empty())
	{
		return;
	}
	const pg::ResourceMapSingletonComponent& resourcesComponent = viewResources.get<const pg::ResourceMapSingletonComponent>(viewResources.front());
	const pg::EngineConfigSingletonComponent& engineConfigComponent = viewEngineConfig.get<const pg::EngineConfigSingletonComponent>(viewEngineConfig.front());
	const pg::ui::RendererConfigSingletonComponent& renderComponent = viewRenderConfig.get<const pg::ui::RendererConfigSingletonComponent>(viewRenderConfig.front());

	auto viewImages = accessor.View<const pg::ui::BaseComponent, const pg::ui::ImageComponent>();
	for (auto ent : viewImages)
	{
		const pg::ui::BaseComponent& baseComponent = viewImages.get<const pg::ui::BaseComponent>(ent);
		if (pg::ui::IsUIElementEnabled(accessor, baseComponent))
		{
			const pg::ui::ImageComponent& imageComponent = viewImages.get<const pg::ui::ImageComponent>(ent);

			const glm::mat4 transform = GetUIElementTransform(accessor, baseComponent, renderComponent, baseComponent.m_Size, baseComponent.m_Size);
			
			pg::DrawUIQuadInFrameEvent quadEvent;
			quadEvent.m_Transform = transform;
			quadEvent.m_TextureID = imageComponent.m_TextureHandle;
			quadEvent.m_Origin = { 0.f,0.f,0.f };
			accessor.EmplaceInframeEvent<pg::DrawUIQuadInFrameEvent>(std::move(quadEvent));
		}
	}

	auto viewText = accessor.View<const pg::ui::BaseComponent, const pg::ui::TextComponent>();
	for (auto ent : viewText)
	{
		const pg::ui::BaseComponent& baseComponent = viewText.get<pg::ui::BaseComponent>(ent);
		if (pg::ui::IsUIElementEnabled(accessor, baseComponent))
		{
			const pg::ui::TextComponent& textComponent = viewText.get<pg::ui::TextComponent>(ent);
			const pg::UUID fontID = textComponent.m_FontID.IsNull() ? engineConfigComponent.m_DefaultFontID : textComponent.m_FontID;
			PG_CORE_EXCEPT(resourcesComponent.m_FontMap.find(fontID) != resourcesComponent.m_FontMap.end(), "could not find font for ui text");
			const unsigned int numLines = GetStringNumLines(textComponent.m_Text, resourcesComponent.m_FontMap.at(fontID));
			const glm::vec2 stringBounds = GetStringBounds(textComponent.m_Text, textComponent.m_Kerning, textComponent.m_Spacing, resourcesComponent.m_FontMap.at(fontID));
			const float fontSize = GetFontSizeFromStringBounds(baseComponent, stringBounds, numLines);
			const glm::mat4 transform = GetUIElementTransform(accessor, baseComponent, renderComponent, glm::vec2(fontSize, fontSize), glm::vec2(stringBounds.x * fontSize, stringBounds.y * fontSize * numLines));
			
			pg::DrawUIStringInFrameEvent stringEvent;
			stringEvent.m_Transform = transform;
			stringEvent.m_String = textComponent.m_Text;
			stringEvent.m_FontID = fontID;
			stringEvent.m_Color = textComponent.m_Color;
			stringEvent.m_Kerning = textComponent.m_Kerning;
			stringEvent.m_Linespacing = textComponent.m_Spacing;
			accessor.EmplaceInframeEvent<pg::DrawUIStringInFrameEvent>(std::move(stringEvent));
		}
	}
}
