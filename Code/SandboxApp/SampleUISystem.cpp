#include "SampleUISystem.h"

#include "Pigeon/ECS/World.h"

pg::SystemAccessDecl sbx::SampleUISystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.writeSet = {
		std::type_index(typeid(pg::ui::BaseComponent)),
		std::type_index(typeid(pg::ui::ImageComponent)),
		std::type_index(typeid(pg::ui::TextComponent)),
		std::type_index(typeid(pg::ui::RendererConfig)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::ui::RendererConfig)),
	};
	return decl;
}

void sbx::SampleUISystem::Update(const pg::Timestep& ts)
{
	auto accessor = pg::World::GetRegistry();
	entt::registry& reg = accessor.GetInternalRegistry();

	auto viewRenderConfig = accessor.view<const pg::ui::RendererConfig>();
	if (viewRenderConfig.size() == 0)
	{
		pg::ui::RendererConfig config;
		config.m_Font = m_Helper->CreateUIFont();
		entt::entity configEntity = accessor.create();
		accessor.emplace_deferred<pg::ui::RendererConfig>(configEntity, std::move(config));
		return;
	}
	PG_CORE_ASSERT(viewRenderConfig.size() == 1, "There should only be one ui render config component");
	const pg::ui::RendererConfig& renderComponent = viewRenderConfig.get<const pg::ui::RendererConfig>(viewRenderConfig.front());

	m_Helper->RendererBeginScene(renderComponent.m_Camera);
	auto viewImages = accessor.view<const pg::ui::BaseComponent, const pg::ui::ImageComponent>();
	for (auto ent : viewImages)
	{
		const pg::ui::BaseComponent& baseComponent = viewImages.get<pg::ui::BaseComponent>(ent);
		if (pg::ui::IsUIElementEnabled(reg, baseComponent))
		{
			const pg::ui::ImageComponent& imageComponent = viewImages.get<pg::ui::ImageComponent>(ent);

			const glm::mat4 transform = GetUIElementTransform(reg, baseComponent, renderComponent, baseComponent.m_Size, baseComponent.m_Size);
			m_Helper->RendererDrawQuad(transform, imageComponent.m_TextureHandle, { 0.f,0.f,0.f });
		}
	}

	auto viewText = accessor.view<const pg::ui::BaseComponent, const pg::ui::TextComponent>();
	for (auto ent : viewText)
	{
		const pg::ui::BaseComponent& baseComponent = viewText.get<pg::ui::BaseComponent>(ent);
		if (pg::ui::IsUIElementEnabled(reg, baseComponent))
		{
			const pg::ui::TextComponent& textComponent = viewText.get<pg::ui::TextComponent>(ent);
			const unsigned int numLines = m_Helper->GetStringNumLines(textComponent.m_Text, renderComponent.m_Font);
			const glm::vec2 stringBounds = m_Helper->GetStringBounds(textComponent.m_Text, textComponent.m_Kerning, textComponent.m_Spacing, renderComponent.m_Font);
			const float fontSize = GetFontSizeFromStringBounds(baseComponent, stringBounds, numLines);
			const glm::mat4 transform = GetUIElementTransform(reg, baseComponent, renderComponent, glm::vec2(fontSize, fontSize), glm::vec2(stringBounds.x * fontSize, stringBounds.y * fontSize * numLines));
			m_Helper->RendererDrawString(transform, textComponent.m_Text, renderComponent.m_Font, textComponent.m_Color, textComponent.m_Kerning, textComponent.m_Spacing);
		}
	}
	m_Helper->RendererEndScene();
}