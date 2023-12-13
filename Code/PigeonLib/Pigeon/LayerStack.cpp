#include "pch.h"
#include "LayerStack.h"

pig::LayerStack::~LayerStack()
{
	Shutdown();
}

void pig::LayerStack::Shutdown()
{
	for (pig::S_Ptr<Layer> layer : m_Data.m_Layers)
		layer->OnDetach();
	for (pig::S_Ptr<Layer> layer : m_Data.m_Layers)
		layer.reset();

	m_Data.m_Layers.clear();
}

void pig::LayerStack::PushLayer(pig::S_Ptr<pig::Layer> layer)
{
	m_Data.m_Layers.emplace(m_Data.m_Layers.begin() + m_Data.m_LayerInsertIndex, layer);
	m_Data.m_LayerInsertIndex++;
}

void pig::LayerStack::PushOverlay(pig::S_Ptr<pig::Layer> overlay)
{
	m_Data.m_Layers.emplace_back(overlay);
}

void pig::LayerStack::PopLayer(pig::S_Ptr<pig::Layer> layer)
{
	auto it = std::find(m_Data.m_Layers.begin(), m_Data.m_Layers.end(), layer);
	if (it != m_Data.m_Layers.end())
	{
		m_Data.m_Layers.erase(it);
		m_Data.m_LayerInsertIndex--;
	}
}

void pig::LayerStack::PopOverlay(pig::S_Ptr<pig::Layer> overlay)
{
	auto it = std::find(m_Data.m_Layers.begin(), m_Data.m_Layers.end(), overlay);
	if (it != m_Data.m_Layers.end())
		m_Data.m_Layers.erase(it);
}