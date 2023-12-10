#include "TestApp.h"

void TestApp::SendFakeEvent(pigeon::WindowsWindow::EventType type, WPARAM wParam, LPARAM lParam)
{
	static_cast<pigeon::WindowsWindow&>(GetWindow()).SendFakeEvent(type, wParam, lParam);
}

pigeon::Application* pigeon::CreateApplication()
{
	pigeon::Log::Init();
	return new TestApp();
}
