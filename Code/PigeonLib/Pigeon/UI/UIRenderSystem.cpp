#include "UIRenderSystem.h"

#include "Pigeon/UI/UIComponents.h"

#include "Pigeon/ECS/World.h"

#include "Pigeon/Renderer/Font.h"
#include <Pigeon/Renderer/Renderer2D.h>

pig::ui::UIRenderSystem::UIRenderSystem(pig::S_Ptr<IUIRenderSystemHelper> helper):
	pig::System(pig::SystemType::eTest),
	m_Helper(helper)
{
}

void pig::ui::UIRenderSystem::Update(float dt)
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
	const pig::ui::RendererConfig& renderComponent = viewRenderConfig.get<pig::ui::RendererConfig>(viewRenderConfig.front());
	
	m_Helper->RendererBeginScene(renderComponent.m_Camera);
	auto viewImages = pig::World::GetRegistry().view<const pig::ui::BaseComponent, const pig::ui::ImageComponent>();
	for (auto ent : viewImages)
	{
		const pig::ui::BaseComponent& baseComponent = viewImages.get<pig::ui::BaseComponent>(ent);
		const pig::ui::ImageComponent& imageComponent = viewImages.get<pig::ui::ImageComponent>(ent);

		const glm::mat4 transform = GetUIElementTransform(baseComponent, renderComponent);
		m_Helper->RendererDrawQuad(transform, imageComponent.m_TextureHandle, { 0.f,0.f,0.f });
	}

	auto viewText = pig::World::GetRegistry().view<const pig::ui::BaseComponent, const pig::ui::TextComponent>();
	for (auto ent : viewText)
	{
		const pig::ui::BaseComponent& baseComponent = viewText.get<pig::ui::BaseComponent>(ent);
		const pig::ui::TextComponent& textComponent = viewText.get<pig::ui::TextComponent>(ent);

		const glm::mat4 transform = GetUIElementTransform(baseComponent, renderComponent);
		m_Helper->RendererDrawString(transform, textComponent.m_Text, renderComponent.m_Font, textComponent.m_Color, textComponent.m_Kerning, textComponent.m_Spacing);
	}
	m_Helper->RendererEndScene();
}

glm::mat4 pig::ui::UIRenderSystem::GetUIElementTransform(const pig::ui::BaseComponent& baseComponent, const pig::ui::RendererConfig& renderComponent) const
{
	glm::mat4 transform(1.f);
	int level = 0;
	const glm::vec4 bounds = GetGlobalBoundsForElement(baseComponent, renderComponent, level);
	glm::vec2 posFinal = glm::vec2(bounds.x, bounds.y);

	if (baseComponent.m_HAlign == EHAlignType::eRight)
	{
		posFinal.x += bounds.z - (baseComponent.m_Size.x + baseComponent.m_Spacing.x);
	}
	else if (baseComponent.m_HAlign == EHAlignType::eCenter)
	{
		posFinal.x += (bounds.z /2.f - baseComponent.m_Size.x/2.f) + baseComponent.m_Spacing.x;
	}
	else
	{
		posFinal.x += baseComponent.m_Spacing.x;
	}

	if (baseComponent.m_VAlign == EVAlignType::eBottom)
	{
		posFinal.y += bounds.w - (baseComponent.m_Size.y + baseComponent.m_Spacing.y);
	}
	else if (baseComponent.m_VAlign == EVAlignType::eCenter)
	{
		posFinal.y += (bounds.w / 2.f - baseComponent.m_Size.y / 2.f) + baseComponent.m_Spacing.y;
	}
	else
	{
		posFinal.y += baseComponent.m_Spacing.y;
	}

	transform = glm::translate(transform, glm::vec3(posFinal, -level * 0.1f));
	transform = glm::scale(transform, glm::vec3(baseComponent.m_Size, 1.f));
	return transform;
}

glm::vec4 pig::ui::UIRenderSystem::GetGlobalBoundsForElement(const pig::ui::BaseComponent& baseComponent, const pig::ui::RendererConfig& renderComponent, int& DEPRECATED_level) const
{
	glm::vec4 globalBounds(0.f, 0.f, renderComponent.m_Width, renderComponent.m_Height);

	if (baseComponent.m_Parent != entt::null && pig::World::GetRegistry().any_of<pig::ui::BaseComponent>(baseComponent.m_Parent))
	{
		const pig::ui::BaseComponent& parentComponent = pig::World::GetRegistry().get<const pig::ui::BaseComponent>(baseComponent.m_Parent);
		DEPRECATED_level += 1;
		globalBounds = GetGlobalBoundsForElement(parentComponent, renderComponent, DEPRECATED_level);

		if (parentComponent.m_HAlign == EHAlignType::eRight)
		{
			globalBounds.x += globalBounds.z - (parentComponent.m_Spacing.x + parentComponent.m_Size.x);
		}
		else if (parentComponent.m_HAlign == EHAlignType::eCenter)
		{
			globalBounds.x += globalBounds.z / 2.f - parentComponent.m_Size.x / 2.f + parentComponent.m_Spacing.x;
		}
		else
		{
			globalBounds.x += parentComponent.m_Spacing.x;
		}

		if (parentComponent.m_VAlign == EVAlignType::eBottom)
		{
			globalBounds.y += globalBounds.w - (parentComponent.m_Spacing.y + parentComponent.m_Size.y);
		}
		else if (parentComponent.m_VAlign == EVAlignType::eCenter)
		{
			globalBounds.y += globalBounds.w / 2.f - parentComponent.m_Size.y / 2.f + parentComponent.m_Spacing.y;
		}
		else
		{
			globalBounds.y += parentComponent.m_Spacing.y;
		}

		globalBounds.z = parentComponent.m_Size.x;
		globalBounds.w = parentComponent.m_Size.y;
	}

	return globalBounds;
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
