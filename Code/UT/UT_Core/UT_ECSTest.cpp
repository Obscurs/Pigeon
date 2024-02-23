#pragma once
#include <catch.hpp>
#include <cstdlib>
#include "Utils/TestApp.h"

#include <Pigeon/ECS/World.h>
#include <Pigeon/ECS/System.h>

#define FLOAT_EQ(x, y) (std::fabs((x) - (y)) < 1e-6)

namespace
{
	static float s_DeltaReceived = 0.f;

	class TestSystem : public pig::System
	{
	public:
		TestSystem()
			: System(pig::SystemType::eTest)
		{

		}
		~TestSystem() = default;
		void Update(float dt)
		{
			s_DeltaReceived += dt;
		}
	};
}
namespace CatchTestsetFail
{
	TEST_CASE("Core.ECS::World")
	{
		pig::World& world = pig::World::Create();

		pig::U_Ptr<TestSystem> testSystem = std::make_unique<TestSystem>();
		world.RegisterSystem(std::move(testSystem));

		world.Update(3.14f);
		CHECK(FLOAT_EQ(s_DeltaReceived, 3.14f));
	}
} // End namespace: CatchTestsetFail

