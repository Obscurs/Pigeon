#pragma once
#include <entt/entt.hpp>
#include "Pigeon/ECS/System.h"

namespace pig
{
	class Font;
	class OrthographicCamera;
}
namespace pig::ui
{
	struct BaseComponent;
	struct RendererConfig;
}

namespace pig::ui
{
	class IUIRenderSystemHelper
	{
	public:
		virtual void RendererBeginScene(const pig::OrthographicCamera& camera) = 0;
		virtual void RendererEndScene() = 0;
		virtual void RendererDrawQuad(const glm::mat4& transform, const pig::UUID& textureID, const glm::vec3& origin) = 0;
		virtual void RendererDrawString(const glm::mat4& transform, const std::string& string, pig::S_Ptr<pig::Font> font, const glm::vec4& color, float kerning, float linespacing) = 0;
		virtual pig::S_Ptr<pig::Font> CreateUIFont() = 0;
	};

	class UIRenderSystemHelper : public IUIRenderSystemHelper
	{
	public:
		virtual void RendererBeginScene(const pig::OrthographicCamera& camera) override;
		virtual void RendererEndScene() override;
		virtual void RendererDrawQuad(const glm::mat4& transform, const pig::UUID& textureID, const glm::vec3& origin) override;
		virtual void RendererDrawString(const glm::mat4& transform, const std::string& string, pig::S_Ptr<pig::Font> font, const glm::vec4& color, float kerning, float linespacing) override;
		virtual pig::S_Ptr<pig::Font> CreateUIFont() override;
	};

	class UIRenderSystem : public pig::System
	{
	public:
		UIRenderSystem(pig::S_Ptr<IUIRenderSystemHelper> helper);
		~UIRenderSystem() = default;
		void Update(float dt) override;

	private:
		glm::mat4 GetUIElementTransform(const pig::ui::BaseComponent& baseComponent, const pig::ui::RendererConfig& renderComponent) const;
		//ARNAU TODO do z render properly instead of using these levels
		glm::vec4 GetGlobalBoundsForElement(const pig::ui::BaseComponent& baseComponent, const pig::ui::RendererConfig& renderComponent, int& DEPRECATED_level) const;

		pig::S_Ptr<IUIRenderSystemHelper> m_Helper;
	};
}