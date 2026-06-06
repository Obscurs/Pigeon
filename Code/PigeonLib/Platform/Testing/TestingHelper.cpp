#include "Platform/Testing/TestingHelper.h"

pg::S_Ptr<pg::TestingHelper> pg::TestingHelper::s_Instance = std::make_shared<pg::TestingHelper>();

void pg::TestingHelper::Reset()
{
	s_Instance = std::make_shared<pg::TestingHelper>();
}

