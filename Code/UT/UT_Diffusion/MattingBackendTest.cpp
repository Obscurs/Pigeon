#include <catch2/catch.hpp>

#include "Pigeon/Diffusion/MattingBackend.h"
#include "Platform/Testing/TestingMattingBackend.h"

namespace
{
	pg::Image SolidImage(uint32_t width, uint32_t height, uint8_t value)
	{
		pg::Image image;
		image.m_Width = width;
		image.m_Height = height;
		image.m_Pixels.assign(static_cast<size_t>(width) * height * 3, value);
		return image;
	}
}

TEST_CASE("Diffusion.MattingBackend::CreateReturnsTestingBackendUnderTestApi")
{
	pg::S_Ptr<pg::MattingBackend> backend = pg::MattingBackend::Create();
	REQUIRE(backend != nullptr);

	// In the Testing render-API the factory must select the deterministic mock, not the real runtime.
	pg::TestingMattingBackend* mock = dynamic_cast<pg::TestingMattingBackend*>(backend.get());
	CHECK(mock != nullptr);
}

TEST_CASE("Diffusion.MattingBackend::NotLoadedBeforeModel")
{
	pg::S_Ptr<pg::MattingBackend> backend = pg::MattingBackend::Create();
	REQUIRE(backend != nullptr);

	CHECK(backend->IsLoaded() == false);

	// Matting with no model yields an empty image (the caller falls back to the skeleton silhouette).
	pg::Image matte = backend->Matte(SolidImage(8, 8, 200));
	CHECK(matte.m_Pixels.empty());
}

TEST_CASE("Diffusion.MattingBackend::LoadModelMarksLoaded")
{
	pg::S_Ptr<pg::MattingBackend> backend = pg::MattingBackend::Create();
	REQUIRE(backend != nullptr);

	const bool ok = backend->LoadModel("some/isnet-general-use.onnx");
	CHECK(ok == true);
	CHECK(backend->IsLoaded() == true);
}

TEST_CASE("Diffusion.MattingBackend::EmptyModelPathDoesNotLoad")
{
	pg::S_Ptr<pg::MattingBackend> backend = pg::MattingBackend::Create();
	REQUIRE(backend != nullptr);

	CHECK(backend->LoadModel("") == false);
	CHECK(backend->IsLoaded() == false);
}

TEST_CASE("Diffusion.MattingBackend::MatteReturnsAlphaOfInputSize")
{
	pg::S_Ptr<pg::MattingBackend> backend = pg::MattingBackend::Create();
	REQUIRE(backend != nullptr);
	REQUIRE(backend->LoadModel("isnet.onnx"));

	pg::Image matte = backend->Matte(SolidImage(16, 12, 200));

	CHECK(matte.m_Width == 16);
	CHECK(matte.m_Height == 12);
	CHECK(matte.m_Pixels.size() == static_cast<size_t>(16) * 12 * 3);
}

TEST_CASE("Diffusion.MattingBackend::MockMatteIsLeftForegroundRightBackground")
{
	pg::S_Ptr<pg::MattingBackend> backend = pg::MattingBackend::Create();
	REQUIRE(backend != nullptr);
	REQUIRE(backend->LoadModel("isnet.onnx"));

	pg::Image matte = backend->Matte(SolidImage(16, 4, 200));

	// Left half foreground (white), right half background (black) — the mock's deterministic split.
	CHECK(static_cast<int>(matte.m_Pixels[0]) == 255);
	const size_t rightIndex = (static_cast<size_t>(0) * 16 + 15) * 3;
	CHECK(static_cast<int>(matte.m_Pixels[rightIndex]) == 0);

	pg::TestingMattingBackend* mock = dynamic_cast<pg::TestingMattingBackend*>(backend.get());
	REQUIRE(mock != nullptr);
	CHECK(mock->GetMatteCount() == 1);
	CHECK(mock->GetModelPath() == "isnet.onnx");
}
