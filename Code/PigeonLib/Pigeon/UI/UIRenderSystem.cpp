#include "UIRenderSystem.h"

#include "Pigeon/UI/UIComponents.h"
#include "Pigeon/UI/UIHelpers.h"

#include "Pigeon/ECS/World.h"

#include "Pigeon/Renderer/Font.h"
#include <Pigeon/Renderer/Renderer2D.h>

pig::ui::UIRenderSystem::UIRenderSystem(pig::S_Ptr<IUIRenderSystemHelper> helper)
	: m_Helper(helper)
{
}

void pig::ui::UIRenderSystem::Update(const pig::Timestep& ts)
{
	auto viewRenderConfig = pig::World::GetRegistry().view<const pig::ui::RendererConfig>();
	if (viewRenderConfig.size() == 0)
	{
		entt::entity configEntity = pig::World::GetRegistry().create();

		pig::ui::RendererConfig& configComponent = pig::World::GetRegistry().emplace<pig::ui::RendererConfig>(configEntity);
		configComponent.m_Font = m_Helper->CreateUIFont();
		return;
	}
	PG_CORE_ASSERT(viewRenderConfig.size() == 1, "There should only be one ui render config component");
	const pig::ui::RendererConfig& renderComponent = viewRenderConfig.get<const pig::ui::RendererConfig>(viewRenderConfig.front());
	
	m_Helper->RendererBeginScene(renderComponent.m_Camera);
	auto viewImages = pig::World::GetRegistry().view<const pig::ui::BaseComponent, const pig::ui::ImageComponent>();
	for (auto ent : viewImages)
	{
		const pig::ui::BaseComponent& baseComponent = viewImages.get<pig::ui::BaseComponent>(ent);
		if (pig::ui::IsUIElementEnabled(baseComponent))
		{
			const pig::ui::ImageComponent& imageComponent = viewImages.get<pig::ui::ImageComponent>(ent);

			const glm::mat4 transform = GetUIElementTransform(baseComponent, renderComponent, baseComponent.m_Size, baseComponent.m_Size);
			m_Helper->RendererDrawQuad(transform, imageComponent.m_TextureHandle, { 0.f,0.f,0.f });
		}
	}

	auto viewText = pig::World::GetRegistry().view<const pig::ui::BaseComponent, const pig::ui::TextComponent>();
	for (auto ent : viewText)
	{
		const pig::ui::BaseComponent& baseComponent = viewText.get<pig::ui::BaseComponent>(ent);
		if (pig::ui::IsUIElementEnabled(baseComponent))
		{
			const pig::ui::TextComponent& textComponent = viewText.get<pig::ui::TextComponent>(ent);
			const unsigned int numLines = m_Helper->GetStringNumLines(textComponent.m_Text, renderComponent.m_Font);
			const glm::vec2 stringBounds = m_Helper->GetStringBounds(textComponent.m_Text, textComponent.m_Kerning, textComponent.m_Spacing, renderComponent.m_Font);
			const float fontSize = GetFontSizeFromStringBounds(baseComponent, stringBounds, numLines);
			const glm::mat4 transform = GetUIElementTransform(baseComponent, renderComponent, glm::vec2(fontSize, fontSize), glm::vec2(stringBounds.x * fontSize, stringBounds.y * fontSize * numLines));
			m_Helper->RendererDrawString(transform, textComponent.m_Text, renderComponent.m_Font, textComponent.m_Color, textComponent.m_Kerning, textComponent.m_Spacing);
		}
	}
	m_Helper->RendererEndScene();
}

glm::mat4 pig::ui::UIRenderSystem::GetUIElementTransform(const pig::ui::BaseComponent& baseComponent, const pig::ui::RendererConfig& renderComponent, const glm::vec2& uiTransformScale, const glm::vec2& uiBoundsSize) const
{
	int level = 0;
	const glm::vec4 bounds = pig::ui::GetGlobalBoundsForElement(baseComponent, renderComponent, uiBoundsSize, level);

	glm::mat4 transform(1.f);
	transform = glm::translate(transform, glm::vec3(bounds.x, bounds.y, -level * 0.1f));
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

void pig::ui::UIRenderSystemHelper::RendererBeginScene(const pig::OrthographicCamera& camera)
{
	//ARNAU TODO Begin end scene in another system??? where do we do that?
	pig::Renderer2D::Clear({ 0.0f, 0.0f, 0.0f, 0.f });
	pig::Renderer2D::BeginScene(camera);
}

void pig::ui::UIRenderSystemHelper::RendererEndScene()
{
	pig::Renderer2D::EndScene();
}

void pig::ui::UIRenderSystemHelper::RendererDrawQuad(const glm::mat4& transform, const pig::UUID& textureID, const glm::vec3& origin)
{
	pig::Renderer2D::DrawQuad(transform, textureID, origin);
}

void pig::ui::UIRenderSystemHelper::RendererDrawString(const glm::mat4& transform, const std::string& string, pig::S_Ptr<pig::Font> font, const glm::vec4& color, float kerning, float linespacing)
{
	pig::Renderer2D::DrawString(transform, string, font, color, kerning, linespacing);
}

pig::S_Ptr<pig::Font> pig::ui::UIRenderSystemHelper::CreateUIFont()
{
	//ARNAU TODO get font path from system/component
	return std::make_shared<pig::Font>("Assets/Fonts/opensans/OpenSans-Regular.ttf");
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
