#pragma once
#include <catch2/catch.hpp>
#include <cstdlib>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

#include "Utils/TestApp.h"

#include <Pigeon/Core/Log.h>
#include <Pigeon/Core/UUID.h>

#define FLOAT_EQ(x, y) (std::fabs((x) - (y)) < 1e-6)

namespace CatchTestsetFail
{
	TEST_CASE("Core.UUID::UUID")
	{
		pig::Application& app = pig::CreateApplication();

		SECTION("Null UUID and default constructor")
		{
			std::string nullUUIDstr("00000000-0000-0000-0000-000000000000");
			pig::UUID uuid;
			CHECK(uuid.IsNull());
			CHECK(uuid.ToString() == nullUUIDstr);
			CHECK(uuid == pig::UUID::s_NullUUID);
			CHECK(uuid == pig::UUID(nullUUIDstr));
		}
		SECTION("Generate random UUID")
		{
			pig::UUID uuid1 = pig::UUID::Generate();
			pig::UUID uuid2 = pig::UUID::Generate();
			CHECK(!uuid1.IsNull());
			CHECK(!uuid2.IsNull());
			CHECK(uuid1 != uuid2);
		}
		SECTION("String constructor")
		{
			std::string stringUUID("12345678-9abc-def0-1234-56789abcdef0");
			std::string stringUUID_mixedCase("12345678-9ABC-DEF0-1234-56789abcdef0");
			pig::UUID uuid1(stringUUID);
			pig::UUID uuid2(stringUUID_mixedCase.c_str());
			CHECK(uuid1.ToString() == stringUUID);
			CHECK(uuid2.ToString() == stringUUID);
			CHECK(uuid1.ToString() == uuid2.ToString());
			CHECK(uuid1 == uuid2);
			CHECK(!uuid1.IsNull());
			CHECK(!uuid2.IsNull());
			pig::UUID uuid3 = pig::UUID::Generate();
			bool eq = uuid3 != uuid1;
			CHECK(uuid3 != uuid1);
		}
		SECTION("Fail invalid strings")
		{
			CHECK_THROWS(pig::UUID("J0000000-0000-0000-0000-000000000000"));
			CHECK_THROWS(pig::UUID("00000000-0000-0000-0000-0000000000000"));
			CHECK_THROWS(pig::UUID("00000000-0000-0000-0000-00000000000"));
			CHECK_THROWS(pig::UUID("00000000000000000000000000000000"));
		}
	}
}