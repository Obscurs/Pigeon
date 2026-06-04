#include "pch.h"
#include "TestingWindow.h"

#include <Pigeon/Events/ApplicationEvent.h>
#include <Pigeon/Events/MouseEvent.h>
#include <Pigeon/Events/KeyEvent.h>

#ifdef TESTS_ENABLED
pg::Window::Data pg::Window::m_Data;
pg::S_Ptr<pg::Window> pg::Window::Create(const pg::WindowProps& props)
{
	return std::make_unique<pg::TestingWindow>(props);
}
#endif

pg::TestingWindow::TestingWindow(const pg::WindowProps& props)
{
}

pg::TestingWindow::~TestingWindow()
{
}

void pg::TestingWindow::Shutdown()
{
	
}

void pg::TestingWindow::TESTING_TriggerEvent(pg::Event* event)
{
	m_Data.EventCallback(*event);
}

void pg::TestingWindow::OnUpdate()
{
	
}