#include <catch2/catch.hpp>

#include "Pigeon/TextGen/TextGenBackend.h"
#include "Platform/Testing/TestingTextGenBackend.h"

TEST_CASE("TextGen.TextGenBackend::CreateReturnsTestingBackendUnderTestApi")
{
	pg::S_Ptr<pg::TextGenBackend> backend = pg::TextGenBackend::Create();
	REQUIRE(backend != nullptr);

	// In the Testing render-API the factory must select the deterministic mock, not the real runtime.
	pg::TestingTextGenBackend* mock = dynamic_cast<pg::TestingTextGenBackend*>(backend.get());
	CHECK(mock != nullptr);
}

TEST_CASE("TextGen.TextGenBackend::NotLoadedBeforeModel")
{
	pg::S_Ptr<pg::TextGenBackend> backend = pg::TextGenBackend::Create();
	REQUIRE(backend != nullptr);

	CHECK(backend->IsLoaded() == false);

	// Generating with no model yields an empty string.
	pg::TextGenParams params;
	params.m_Prompt = "hello";
	CHECK(backend->Generate(params).empty());
}

TEST_CASE("TextGen.TextGenBackend::LoadModelMarksLoaded")
{
	pg::S_Ptr<pg::TextGenBackend> backend = pg::TextGenBackend::Create();
	REQUIRE(backend != nullptr);

	const bool ok = backend->LoadModel("some/model.gguf", 999);
	CHECK(ok == true);
	CHECK(backend->IsLoaded() == true);
}

TEST_CASE("TextGen.TextGenBackend::EmptyModelPathDoesNotLoad")
{
	pg::S_Ptr<pg::TextGenBackend> backend = pg::TextGenBackend::Create();
	REQUIRE(backend != nullptr);

	CHECK(backend->LoadModel("", 999) == false);
	CHECK(backend->IsLoaded() == false);
}

TEST_CASE("TextGen.TextGenBackend::MockRecordsGpuLayersFromLoad")
{
	pg::S_Ptr<pg::TextGenBackend> backend = pg::TextGenBackend::Create();
	REQUIRE(backend != nullptr);

	// The GPU-offload depth (n_gpu_layers) is a model-load parameter; the mock records the value it was
	// asked to load with so the config -> LoadModel plumbing is testable without real CUDA inference.
	REQUIRE(backend->LoadModel("model.gguf", 24));

	pg::TestingTextGenBackend* mock = dynamic_cast<pg::TestingTextGenBackend*>(backend.get());
	REQUIRE(mock != nullptr);
	CHECK(mock->GetGpuLayers() == 24);
}

TEST_CASE("TextGen.TextGenBackend::GenerateReturnsNonEmptyWhenLoaded")
{
	pg::S_Ptr<pg::TextGenBackend> backend = pg::TextGenBackend::Create();
	REQUIRE(backend != nullptr);
	REQUIRE(backend->LoadModel("model.gguf", 999));

	pg::TextGenParams params;
	params.m_Prompt = "a pigeon knight";
	const std::string result = backend->Generate(params);

	CHECK(!result.empty());
	CHECK(result.find("a pigeon knight") != std::string::npos);
}

TEST_CASE("TextGen.TextGenBackend::MockRecordsLastGenerateParams")
{
	pg::S_Ptr<pg::TextGenBackend> backend = pg::TextGenBackend::Create();
	REQUIRE(backend != nullptr);
	REQUIRE(backend->LoadModel("model.gguf", 999));

	pg::TextGenParams params;
	params.m_Prompt = "tell me a tale";
	params.m_MaxTokens = 42;
	backend->Generate(params);

	pg::TestingTextGenBackend* mock = dynamic_cast<pg::TestingTextGenBackend*>(backend.get());
	REQUIRE(mock != nullptr);
	CHECK(mock->GetGenerateCount() == 1);
	CHECK(mock->GetLastParams().m_Prompt == "tell me a tale");
	CHECK(mock->GetLastParams().m_MaxTokens == 42);
	CHECK(mock->GetModelPath() == "model.gguf");
}
