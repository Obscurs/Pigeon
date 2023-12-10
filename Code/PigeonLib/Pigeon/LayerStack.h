#pragma once

#include "Pigeon/Core.h"
#include "Layer.h"

#include <vector>

namespace pigeon 
{
	class PIGEON_API LayerStack
	{
	public:
		struct Data
		{
			std::vector<Layer*> m_Layers;
			unsigned int m_LayerInsertIndex = 0;
		};

		LayerStack() = default;
		~LayerStack();

		void Shutdown();
#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
#endif

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* overlay);

		std::vector<Layer*>::iterator begin() { return m_Data.m_Layers.begin(); }
		std::vector<Layer*>::iterator end() { return m_Data.m_Layers.end(); }
	private:
		Data m_Data;
	};
}