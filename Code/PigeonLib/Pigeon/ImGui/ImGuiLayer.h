#pragma once

#include "Pigeon/Layer.h"

#include "Pigeon/Events/ApplicationEvent.h"
#include "Pigeon/Events/KeyEvent.h"
#include "Pigeon/Events/MouseEvent.h"

namespace pigeon 
{
	class PIGEON_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		virtual void Begin() override;
		virtual void End() override;
		bool IsAttached() { return m_Attached; }
	private:
		bool m_Attached;
	};
}