#pragma once

#include "Pigeon/Core.h"
#include "Layer.h"

#include <vector>

namespace pig 
{
	class PIGEON_API LayerStack
	{
	public:
		struct Data
		{
			std::vector<pig::S_Ptr<Layer>> m_Layers;
			unsigned int m_LayerInsertIndex = 0;
		};

		LayerStack() = default;
		~LayerStack();

		void Shutdown();
#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
#endif

		void PushLayer(pig::S_Ptr<Layer> layer);
		void PushOverlay(pig::S_Ptr<Layer> overlay);
		void PopLayer(pig::S_Ptr<Layer> layer);
		void PopOverlay(pig::S_Ptr<Layer> overlay);

		std::vector<pig::S_Ptr<Layer>>::iterator begin() { return m_Data.m_Layers.begin(); }
		std::vector<pig::S_Ptr<Layer>>::iterator end() { return m_Data.m_Layers.end(); }

		std::vector<pig::S_Ptr<Layer>>::reverse_iterator rbegin() { return m_Data.m_Layers.rbegin(); }
		std::vector<pig::S_Ptr<Layer>>::reverse_iterator rend() { return m_Data.m_Layers.rend(); }
	private:
		Data m_Data;
	};
}