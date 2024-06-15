#pragma once
#include "Pigeon/ECS/System.h"

//ARNAU TODO: try to remove this include
#include "Pigeon/UI/UIComponents.h"

namespace pig::ui
{
	class UILayoutSystemInterface : public pig::System
	{
	public:
		UILayoutSystemInterface();
		~UILayoutSystemInterface() = default;
		void Update(float dt) override;
	private:
		void LoadLayout(const std::string& path);
		void UpdateLayoutTransform(const pig::UUID& layoutId, const glm::vec2& size, const glm::vec2& spacing, pig::ui::EHAlignType hvalue, pig::ui::EVAlignType vvalue);
		void UpdateLayoutUUID(const pig::UUID& layoutId, const pig::UUID& value);
		void UpdateLayoutParent(const pig::UUID& layoutId, entt::entity value);
		void UpdateLayoutTexture(const pig::UUID& layoutId, const pig::UUID& value, bool destroyPrevious);
		void UpdateLayoutText(const pig::UUID& layoutId, const std::string& value, const glm::vec4& color);
		void UpdateEnable(const pig::UUID& layoutId, bool value);
		void DestroyLayout(const pig::UUID& layoutId);
	};
}