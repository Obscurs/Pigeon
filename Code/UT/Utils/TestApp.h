#include <Pigeon.h>
#include <Platform/Windows/WindowsWindow.h>

class TestApp : public pigeon::Application
{
public:
	TestApp() = default;
	~TestApp() = default;

	void SendFakeEvent(pigeon::WindowsWindow::EventType type, WPARAM wParam, LPARAM lParam);
};