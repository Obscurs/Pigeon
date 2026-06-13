#include <catch2/catch.hpp>

#include "Pigeon/Diffusion/DiffusionBackend.h"
#include "Platform/Testing/TestingDiffusionBackend.h"

TEST_CASE("Diffusion.DiffusionBackend::CreateReturnsTestingBackendUnderTestApi")
{
	pg::S_Ptr<pg::DiffusionBackend> backend = pg::DiffusionBackend::Create();
	REQUIRE(backend != nullptr);

	// In the Testing render-API the factory must select the deterministic mock, not the real runtime.
	pg::TestingDiffusionBackend* mock = dynamic_cast<pg::TestingDiffusionBackend*>(backend.get());
	CHECK(mock != nullptr);
}

TEST_CASE("Diffusion.DiffusionBackend::NotLoadedBeforeCheckpoint")
{
	pg::S_Ptr<pg::DiffusionBackend> backend = pg::DiffusionBackend::Create();
	REQUIRE(backend != nullptr);

	CHECK(backend->IsLoaded() == false);

	// Generating with no checkpoint yields an empty image.
	pg::DiffusionJobParams params;
	params.m_Width = 8;
	params.m_Height = 8;
	pg::Image result = backend->Generate(params);
	CHECK(result.m_Pixels.empty());
}

TEST_CASE("Diffusion.DiffusionBackend::LoadCheckpointMarksLoaded")
{
	pg::S_Ptr<pg::DiffusionBackend> backend = pg::DiffusionBackend::Create();
	REQUIRE(backend != nullptr);

	const bool ok = backend->LoadCheckpoint("some/checkpoint.safetensors", "some/control.safetensors", "some/vae.safetensors");
	CHECK(ok == true);
	CHECK(backend->IsLoaded() == true);
}

TEST_CASE("Diffusion.DiffusionBackend::EmptyCheckpointPathDoesNotLoad")
{
	pg::S_Ptr<pg::DiffusionBackend> backend = pg::DiffusionBackend::Create();
	REQUIRE(backend != nullptr);

	CHECK(backend->LoadCheckpoint("", "", "") == false);
	CHECK(backend->IsLoaded() == false);
}

TEST_CASE("Diffusion.DiffusionBackend::GenerateReturnsImageOfRequestedSize")
{
	pg::S_Ptr<pg::DiffusionBackend> backend = pg::DiffusionBackend::Create();
	REQUIRE(backend != nullptr);
	REQUIRE(backend->LoadCheckpoint("ckpt.safetensors", "", ""));

	pg::DiffusionJobParams params;
	params.m_Width = 16;
	params.m_Height = 12;
	pg::Image result = backend->Generate(params);

	CHECK(result.m_Width == 16);
	CHECK(result.m_Height == 12);
	CHECK(result.m_Pixels.size() == static_cast<size_t>(16) * 12 * 3);
}

TEST_CASE("Diffusion.DiffusionBackend::MockRecordsLastGenerateParams")
{
	pg::S_Ptr<pg::DiffusionBackend> backend = pg::DiffusionBackend::Create();
	REQUIRE(backend != nullptr);
	REQUIRE(backend->LoadCheckpoint("ckpt.safetensors", "", ""));

	pg::DiffusionJobParams params;
	params.m_Prompt = "a pigeon knight";
	params.m_Width = 8;
	params.m_Height = 8;
	params.m_Loras.push_back(pg::DiffusionLora{ "lin.safetensors", 0.7f });
	backend->Generate(params);

	pg::TestingDiffusionBackend* mock = dynamic_cast<pg::TestingDiffusionBackend*>(backend.get());
	REQUIRE(mock != nullptr);
	CHECK(mock->GetGenerateCount() == 1);
	CHECK(mock->GetLastParams().m_Prompt == "a pigeon knight");
	REQUIRE(mock->GetLastParams().m_Loras.size() == 1);
	CHECK(mock->GetLastParams().m_Loras[0].m_Weight == Approx(0.7f));
}
