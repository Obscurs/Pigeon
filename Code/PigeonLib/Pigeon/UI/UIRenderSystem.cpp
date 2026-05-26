#include "UIRenderSystem.h"

#include "Pigeon/UI/UIComponents.h"
#include "Pigeon/UI/UIHelpers.h"

#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/AddUIFontTextureInFrameEvent.h"
#include "Pigeon/Renderer/DrawUIQuadInFrameEvent.h"
#include "Pigeon/Renderer/DrawUIStringInFrameEvent.h"
#include "Pigeon/Renderer/Font.h"

pig::ui::UIRenderSystem::UIRenderSystem(pig::S_Ptr<IUIRenderSystemHelper> helper)
	: m_Helper(helper)
{
}

pig::SystemAccessDecl pig::ui::UIRenderSystem::DeclareAccess() const
{
	pig::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pig::ui::BaseComponent)),
		std::type_index(typeid(pig::ui::ImageComponent)),
		std::type_index(typeid(pig::ui::TextComponent)),
		std::type_index(typeid(pig::ui::RendererConfig)),
	};
	decl.addSet = {
		std::type_index(typeid(pig::ui::RendererConfig)),
		std::type_index(typeid(pig::AddUIFontTextureInFrameEvent)),
	};
	decl.inframeAddSet = {
		std::type_index(typeid(pig::DrawUIQuadInFrameEvent)),
		std::type_index(typeid(pig::DrawUIStringInFrameEvent)),
	};
	return decl;
}

void pig::ui::UIRenderSystem::Update(const pig::Timestep& ts)
{
	auto accessor = pig::World::GetRegistry();

	auto viewRenderConfig = accessor.view<const pig::ui::RendererConfig>();
	if (viewRenderConfig.size() == 0)
	{
		pig::ui::RendererConfig config;
		pig::AddUIFontTextureInFrameEvent textureEvent;
		config.m_Font = std::make_shared<pig::Font>("Assets/Fonts/opensans/OpenSans-Regular.ttf", textureEvent.m_TextureData);
		entt::entity configEntity = accessor.create();
		accessor.emplace_deferred<pig::ui::RendererConfig>(configEntity, std::move(config));
		accessor.EmplaceEvent<pig::AddUIFontTextureInFrameEvent>(std::move(textureEvent));
		return;
	}
	PG_CORE_ASSERT(viewRenderConfig.size() == 1, "There should only be one ui render config component");
	const pig::ui::RendererConfig& renderComponent = viewRenderConfig.get<const pig::ui::RendererConfig>(viewRenderConfig.front());

	auto viewImages = accessor.view<const pig::ui::BaseComponent, const pig::ui::ImageComponent>();
	for (auto ent : viewImages)
	{
		const pig::ui::BaseComponent& baseComponent = viewImages.get<pig::ui::BaseComponent>(ent);
		if (pig::ui::IsUIElementEnabled(accessor, baseComponent))
		{
			const pig::ui::ImageComponent& imageComponent = viewImages.get<pig::ui::ImageComponent>(ent);

			const glm::mat4 transform = GetUIElementTransform(accessor, baseComponent, renderComponent, baseComponent.m_Size, baseComponent.m_Size);
			
			pig::DrawUIQuadInFrameEvent quadEvent;
			quadEvent.m_Transform = transform;
			quadEvent.m_TextureID = imageComponent.m_TextureHandle;
			quadEvent.m_Origin = { 0.f,0.f,0.f };
			accessor.EmplaceInframeEvent<pig::DrawUIQuadInFrameEvent>(std::move(quadEvent));
		}
	}

	auto viewText = accessor.view<const pig::ui::BaseComponent, const pig::ui::TextComponent>();
	for (auto ent : viewText)
	{
		const pig::ui::BaseComponent& baseComponent = viewText.get<pig::ui::BaseComponent>(ent);
		if (pig::ui::IsUIElementEnabled(accessor, baseComponent))
		{
			const pig::ui::TextComponent& textComponent = viewText.get<pig::ui::TextComponent>(ent);
			const unsigned int numLines = m_Helper->GetStringNumLines(textComponent.m_Text, renderComponent.m_Font);
			const glm::vec2 stringBounds = m_Helper->GetStringBounds(textComponent.m_Text, textComponent.m_Kerning, textComponent.m_Spacing, renderComponent.m_Font);
			const float fontSize = GetFontSizeFromStringBounds(baseComponent, stringBounds, numLines);
			const glm::mat4 transform = GetUIElementTransform(accessor, baseComponent, renderComponent, glm::vec2(fontSize, fontSize), glm::vec2(stringBounds.x * fontSize, stringBounds.y * fontSize * numLines));
			
			pig::DrawUIStringInFrameEvent stringEvent;
			stringEvent.m_Transform = transform;
			stringEvent.m_String = textComponent.m_Text;
			stringEvent.m_Font = renderComponent.m_Font;
			stringEvent.m_Color = textComponent.m_Color;
			stringEvent.m_Kerning = textComponent.m_Kerning;
			stringEvent.m_Linespacing = textComponent.m_Spacing;
			accessor.EmplaceInframeEvent<pig::DrawUIStringInFrameEvent>(std::move(stringEvent));
		}
	}
}

glm::mat4 pig::ui::UIRenderSystem::GetUIElementTransform(pig::CheckedRegistryAccessor& accessor, const pig::ui::BaseComponent& baseComponent, const pig::ui::RendererConfig& renderComponent, const glm::vec2& uiTransformScale, const glm::vec2& uiBoundsSize) const
{
	int level = 0;
	const glm::vec4 bounds = pig::ui::GetGlobalBoundsForElement(accessor, baseComponent, renderComponent, uiBoundsSize, level);

	glm::mat4 transform(1.f);
	transform = glm::translate(transform, glm::vec3(bounds.x, bounds.y, level));
	transform = glm::scale(transform, glm::vec3(uiTransformScale, 1.f));
	return transform;
}

float pig::ui::UIRenderSystem::GetFontSizeFromStringBounds(const pig::ui::BaseComponent& baseComponent, const glm::vec2 stringBounds, unsigned int numLines) const
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

glm::vec2 pig::ui::UIRenderSystemHelper::GetStringBounds(const std::string& string, float kerning, float linespace, pig::S_Ptr<pig::Font> font)
{
	return font->GetStringBounds(string, kerning, linespace);
}

unsigned int pig::ui::UIRenderSystemHelper::GetStringNumLines(const std::string& string, pig::S_Ptr<pig::Font> font)
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
