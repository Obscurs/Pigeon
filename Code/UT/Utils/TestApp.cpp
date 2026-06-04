#include "TestApp.h"

#include <Platform/Testing/TestingHelper.h>

pg::Application& pg::CreateApplication()
{
	pg::Log::Init();
	pg::TestingHelper::Reset();
	return TestApp::Create();
}
