#include <catch2/catch.hpp>

#include "Pigeon/Renderer/PerspectiveCamera.h"

#include <glm/glm.hpp>

namespace
{
	// Projects a world point through the camera and performs the perspective divide, returning the
	// normalized device coordinate (clip xyz / w).
	glm::vec3 ProjectToNDC(const pg::PerspectiveCamera& camera, const glm::vec3& worldPoint)
	{
		const glm::vec4 clip = camera.GetViewProjectionMatrix() * glm::vec4(worldPoint, 1.f);
		return glm::vec3(clip) / clip.w;
	}
}

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// The look-at target projects to the centre of the screen (NDC x = y = 0).
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.PerspectiveCamera::TargetProjectsToScreenCentre")
	{
		pg::PerspectiveCamera camera(glm::radians(45.f), 1.f, 0.1f, 100.f);
		camera.SetView(glm::vec3(0.f, 0.f, -5.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

		const glm::vec3 ndc = ProjectToNDC(camera, glm::vec3(0.f, 0.f, 0.f));

		CHECK(ndc.x == Approx(0.f).margin(1e-5));
		CHECK(ndc.y == Approx(0.f).margin(1e-5));
	}

	// ---------------------------------------------------------------------------
	// Nearer geometry has a smaller (closer-to-zero) NDC depth than farther
	// geometry: zero-to-one depth, so depth testing keeps near faces in front.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.PerspectiveCamera::NearerGeometryHasSmallerDepth")
	{
		pg::PerspectiveCamera camera(glm::radians(45.f), 1.f, 0.1f, 100.f);
		camera.SetView(glm::vec3(0.f, 0.f, -5.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

		// Looking down +Z (eye at z=-5): the point at z=-1 is nearer the eye than the point at z=+1.
		const glm::vec3 nearNDC = ProjectToNDC(camera, glm::vec3(0.f, 0.f, -1.f));
		const glm::vec3 farNDC = ProjectToNDC(camera, glm::vec3(0.f, 0.f, 1.f));

		CHECK(nearNDC.z < farNDC.z);
		CHECK(nearNDC.z >= 0.f);
		CHECK(farNDC.z <= 1.f);
	}

	// ---------------------------------------------------------------------------
	// A point up in the world projects above screen centre (positive NDC y), and a
	// point to the right projects right (positive NDC x): orientation is consistent.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.PerspectiveCamera::PreservesUpAndRightOrientation")
	{
		pg::PerspectiveCamera camera(glm::radians(45.f), 1.f, 0.1f, 100.f);
		camera.SetView(glm::vec3(0.f, 0.f, -5.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

		const glm::vec3 up = ProjectToNDC(camera, glm::vec3(0.f, 1.f, 0.f));
		CHECK(up.y > 0.f);

		const glm::vec3 right = ProjectToNDC(camera, glm::vec3(1.f, 0.f, 0.f));
		CHECK(right.x > 0.f);
	}

	// ---------------------------------------------------------------------------
	// Changing the projection updates the matrix: a wider FOV shrinks the projected
	// extent of the same world point (it moves toward screen centre).
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.PerspectiveCamera::WiderFovShrinksProjectedExtent")
	{
		pg::PerspectiveCamera narrow(glm::radians(30.f), 1.f, 0.1f, 100.f);
		narrow.SetView(glm::vec3(0.f, 0.f, -5.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

		pg::PerspectiveCamera wide(glm::radians(90.f), 1.f, 0.1f, 100.f);
		wide.SetView(glm::vec3(0.f, 0.f, -5.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

		const float narrowX = ProjectToNDC(narrow, glm::vec3(1.f, 0.f, 0.f)).x;
		const float wideX = ProjectToNDC(wide, glm::vec3(1.f, 0.f, 0.f)).x;

		CHECK(wideX < narrowX);
	}
}
