#include "pch.h"
#include "LayerStack.h"

namespace pigeon 
{
	LayerStack::~LayerStack()
	{
		Shutdown();
	}

	void LayerStack::Shutdown()
	{
		for (Layer* layer : m_Data.m_Layers)
			layer->OnDetach();
		for (Layer* layer : m_Data.m_Layers)
			if(layer)
				delete layer;

		m_Data.m_Layers.clear();
	}

	void LayerStack::PushLayer(Layer* layer)
	{
		m_Data.m_Layers.emplace(m_Data.m_Layers.begin() + m_Data.m_LayerInsertIndex, layer);
		m_Data.m_LayerInsertIndex++;
	}

	void LayerStack::PushOverlay(Layer* overlay)
	{
		m_Data.m_Layers.emplace_back(overlay);
	}

	void LayerStack::PopLayer(Layer* layer)
	{
		auto it = std::find(m_Data.m_Layers.begin(), m_Data.m_Layers.end(), layer);
		if (it != m_Data.m_Layers.end())
		{
			m_Data.m_Layers.erase(it);
			m_Data.m_LayerInsertIndex--;
		}
	}

	void LayerStack::PopOverlay(Layer* overlay)
	{
		auto it = std::find(m_Data.m_Layers.begin(), m_Data.m_Layers.end(), overlay);
		if (it != m_Data.m_Layers.end())
			m_Data.m_Layers.erase(it);
	}
}