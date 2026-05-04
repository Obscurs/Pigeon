#pragma once
namespace pig
{
	struct TestComponent
	{
		int m_X;
		int m_Y;

		TestComponent() = default;
		TestComponent(const TestComponent&) = default;
		TestComponent(int x, int y) : m_X(x), m_Y(y) {}
	};
}