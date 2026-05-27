#pragma once
#include <entt/entt.hpp>
#include "Pigeon/ECS/System.h"

namespace pig
{
	class CheckedRegistryAccessor;
	class Font;
	class OrthographicCamera;
	struct TextureData;
}
namespace pig::ui
{
	struct BaseComponent;
	struct RendererConfig;
	struct TextComponent;
}

namespace pig::ui
{
	class IUIRenderSystemHelper
	{
	public:
		virtual glm::vec2 GetStringBounds(const std::string& string, float kerning, float linespace, pig::S_Ptr<pig::Font> font) = 0;
		virtual unsigned int GetStringNumLines(const std::string& string, pig::S_Ptr<pig::Font> font) = 0;
	};

	class UIRenderSystemHelper : public IUIRenderSystemHelper
	{
	public:
		virtual glm::vec2 GetStringBounds(const std::string& string, float kerning, float linespace, pig::S_Ptr<pig::Font> font) override;
		virtual unsigned int GetStringNumLines(const std::string& string, pig::S_Ptr<pig::Font> font) override;
	};

	class UIRenderSystem : public pig::System
	{
	public:
		UIRenderSystem(pig::S_Ptr<IUIRenderSystemHelper> helper);
		~UIRenderSystem() = default;
		void Update(const pig::Timestep& ts) override;
		pig::SystemAccessDecl DeclareAccess() const override;

	private:
		float GetFontSizeFromStringBounds(const pig::ui::BaseComponent& baseComponent, const glm::vec2 stringBounds, unsigned int numLines) const;
		glm::mat4 GetUIElementTransform(pig::CheckedRegistryAccessor& accessor, const pig::ui::BaseComponent& baseComponent, const pig::ui::RendererConfig& renderComponent, const glm::vec2& uiTransformScale, const glm::vec2& uiBoundsSize) const;
		//ARNAU TODO render UI in over the scene by using rendered scene as a texture (render targets and render buffers)

		pig::S_Ptr<IUIRenderSystemHelper> m_Helper;
	};
}