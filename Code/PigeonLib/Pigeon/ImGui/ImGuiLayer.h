#pragma once

#include "Pigeon/Layer.h"

namespace pigeon 
{
	class PIGEON_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void OnAttach();
		void OnDetach();
		void OnUpdate();
		void OnEvent(Event& event);
	};
}