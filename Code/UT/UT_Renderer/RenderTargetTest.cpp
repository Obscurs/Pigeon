#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Renderer/RenderTarget.h"
#include "Pigeon/Renderer/Texture.h"

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// A render target reports its dimensions and exposes a sampleable colour
	// texture (the surface the 2D pass draws the 3D image from).
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.RenderTarget::CreateExposesColorTexture")
	{
		pg::S_Ptr<pg::RenderTarget> renderTarget = pg::RenderTarget::Create(256, 128);
		REQUIRE(renderTarget != nullptr);

		CHECK(renderTarget->GetWidth() == 256);
		CHECK(renderTarget->GetHeight() == 128);
		CHECK(renderTarget->GetColorTexture() != nullptr);
	}
}
