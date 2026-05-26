#pragma once
#include <entt/entt.hpp>
#include "Pigeon/ECS/System.h"

namespace pig
{
	class CheckedRegistryAccessor;
}

namespace pig::ui
{
	class BaseComponent;
	class ImageComponent;
	class TextComponent;

	class UIControlSystem : public pig::System
	{
	public:
		UIControlSystem() = default;
		~UIControlSystem() = default;
		void Update(const pig::Timestep& ts) override;
		pig::SystemAccessDecl DeclareAccess() const override;
	private:
		void LoadLayoutFromFile(const std::string& path);
		void ParseJsonUIElement(pig::CheckedRegistryAccessor& accessor, const json& jsonObject, entt::entity parent);
		void ParseBaseComponentFromJson(pig::CheckedRegistryAccessor& accessor, const json& jsonObject, entt::entity ent, entt::entity parent);
		void ParseImageComponentFromJson(pig::CheckedRegistryAccessor& accessor, const json& jsonObject, entt::entity ent);
		void ParseTextComponentFromJson(pig::CheckedRegistryAccessor& accessor, const json& jsonObject, entt::entity ent);
	};
}