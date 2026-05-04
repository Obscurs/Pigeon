#include "pch.h"
#include "TestingWindow.h"

#include <Pigeon/Events/ApplicationEvent.h>
#include <Pigeon/Events/MouseEvent.h>
#include <Pigeon/Events/KeyEvent.h>

#ifdef TESTS_ENABLED
pig::Window::Data pig::Window::m_Data;
pig::S_Ptr<pig::Window> pig::Window::Create(const pig::WindowProps& props)
{
	return std::make_unique<pig::TestingWindow>(props);
}
#endif

pig::TestingWindow::TestingWindow(const pig::WindowProps& props)
{
}

pig::TestingWindow::~TestingWindow()
{
}

void pig::TestingWindow::Shutdown()
{
	
}

void pig::TestingWindow::TESTING_TriggerEvent(pig::Event* event)
{
	m_Data.EventCallback(*event);
}

void pig::TestingWindow::OnUpdate()
{
	
}