#pragma once
#include <entt/entt.hpp>
#include "Pigeon/ECS/System.h"

namespace pig::ui
{
	class BaseComponent;
	class ImageComponent;
	class TextComponent;

	class IUIControlSystemHelper
	{
	public:
		virtual pig::UUID CreateUIImageFromPath(const std::string& path) = 0;
	};

	class UIControlSystemHelper : public IUIControlSystemHelper
	{
	public:
		virtual pig::UUID CreateUIImageFromPath(const std::string& path) override;
	};

	class UIControlSystem : public pig::System
	{
	public:
		UIControlSystem(pig::S_Ptr<IUIControlSystemHelper> helper);
		~UIControlSystem() = default;
		void Update(const pig::Timestep& ts) override;
	private:
		//ARNAU TODO: this clean should not be in this system, rethink
		void CleanOneFrameComponents();

		void LoadLayoutFromFile(const std::string& path);
		void ParseJsonUIElement(const json& jsonObject, entt::entity parent);
		void ParseBaseComponentFromJson(const json& jsonObject, entt::entity ent, entt::entity parent);
		void ParseImageComponentFromJson(const json& jsonObject, entt::entity ent);
		void ParseTextComponentFromJson(const json& jsonObject, entt::entity ent);

		void DestroyUI(entt::entity ent);

		pig::S_Ptr<IUIControlSystemHelper> m_Helper;
	};
}