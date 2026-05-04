#include "TestingHelper.h"

pig::S_Ptr<pig::TestingHelper> pig::TestingHelper::s_Instance = std::make_shared<pig::TestingHelper>();

void pig::TestingHelper::Reset()
{
	s_Instance = std::make_shared<pig::TestingHelper>();
}

