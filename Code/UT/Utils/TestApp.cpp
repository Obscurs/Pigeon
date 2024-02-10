#include "TestApp.h"

pig::Application& pig::CreateApplication()
{
	pig::Log::Init();
	return TestApp::Create();
}
