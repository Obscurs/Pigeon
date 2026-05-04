#pragma once
#include <catch2/catch.hpp>
#include <cstdlib>
#include "Utils/TestApp.h"

#include <Pigeon/ECS/World.h>
#include <Pigeon/ECS/System.h>

#define FLOAT_EQ(x, y) (std::fabs((x) - (y)) < 1e-6)

namespace
{
	static uint64_t s_DeltaReceived = 0.f;

	class TestSystem : public pig::System
	{
	public:
		TestSystem() = default;
		~TestSystem() = default;
		void Update(const pig::Timestep& ts)
		{
			s_DeltaReceived += ts.AsMilliseconds();
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
		pig::Timestep ts(3000);
		world.Update(ts);
		CHECK(s_DeltaReceived == 3000);
	}
} // End namespace: CatchTestsetFail

