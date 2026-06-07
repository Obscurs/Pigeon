#pragma once

namespace sbx
{
	// Present once SceneSetupSystem has built the persistent scene (camera, sprite, labels and
	// the UI layout request). Acts as the system's run-once guard.
	struct SceneReadySingletonComponent
	{
		SceneReadySingletonComponent() = default;
		SceneReadySingletonComponent(const SceneReadySingletonComponent&) = default;

		bool m_Dummy = true;
	};
}
