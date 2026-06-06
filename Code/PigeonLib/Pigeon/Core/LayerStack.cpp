#include "pch.h"
#include "Pigeon/Core/LayerStack.h"

pg::LayerStack::~LayerStack()
{
	Shutdown();
}

void pg::LayerStack::Shutdown()
{
	for (pg::S_Ptr<Layer> layer : m_Data.m_Layers)
	{
		layer->OnDetach();
	}
	m_Data.m_Layers.clear();
}

void pg::LayerStack::PushLayer(pg::S_Ptr<pg::Layer> layer)
{
	m_Data.m_Layers.emplace(m_Data.m_Layers.begin() + m_Data.m_LayerInsertIndex, layer);
	m_Data.m_LayerInsertIndex++;
}

void pg::LayerStack::PushOverlay(pg::S_Ptr<pg::Layer> overlay)
{
	m_Data.m_Layers.emplace_back(overlay);
}

void pg::LayerStack::PopLayer(pg::S_Ptr<pg::Layer> layer)
{
	auto it = std::find(m_Data.m_Layers.begin(), m_Data.m_Layers.end(), layer);
	if (it != m_Data.m_Layers.end())
	{
		m_Data.m_Layers.erase(it);
		m_Data.m_LayerInsertIndex--;
	}
}

void pg::LayerStack::PopOverlay(pg::S_Ptr<pg::Layer> overlay)
{
	auto it = std::find(m_Data.m_Layers.begin(), m_Data.m_Layers.end(), overlay);
	if (it != m_Data.m_Layers.end())
		m_Data.m_Layers.erase(it);
}