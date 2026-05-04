#include "TestApp.h"

#include <Platform/Testing/TestingHelper.h>

pig::Application& pig::CreateApplication()
{
	pig::Log::Init();
	pig::TestingHelper::Reset();
	return TestApp::Create();
}
