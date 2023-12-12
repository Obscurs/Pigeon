#include "TestApp.h"

pig::S_Ptr<pig::Application> pig::CreateApplication()
{
	pig::Log::Init();
	return TestApp::Create();
}
