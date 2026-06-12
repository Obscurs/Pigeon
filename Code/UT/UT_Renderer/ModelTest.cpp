#pragma once
#include <catch2/catch.hpp>

#include "Pigeon/Renderer/Model.h"

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Happy path: a one-quad .obj parses into four unique vertices and a
	// fan-triangulated index buffer of six indices.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Model::ParsesQuadIntoTrianglesAndUniqueVertices")
	{
		pg::S_Ptr<pg::Model> model = pg::Model::Create("Assets/UT/Models/quad.obj");
		REQUIRE(model != nullptr);

		// One quad face -> four distinct v/vt/vn corners -> four vertices.
		CHECK(model->GetVertices().size() == 4);
		// The quad is fan-triangulated into two triangles -> six indices.
		REQUIRE(model->GetIndices().size() == 6);

		const std::vector<uint32_t>& indices = model->GetIndices();
		CHECK(indices[0] == 0);
		CHECK(indices[1] == 1);
		CHECK(indices[2] == 2);
		CHECK(indices[3] == 0);
		CHECK(indices[4] == 2);
		CHECK(indices[5] == 3);
	}

	// ---------------------------------------------------------------------------
	// Happy path: vertex attributes (position, normal, texcoord) are read from the
	// v / vn / vt lines and assembled per face corner.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Model::AssemblesVertexAttributes")
	{
		pg::S_Ptr<pg::Model> model = pg::Model::Create("Assets/UT/Models/quad.obj");
		REQUIRE(model != nullptr);
		REQUIRE(model->GetVertices().size() == 4);

		const pg::ModelVertex& first = model->GetVertices()[0];
		CHECK(first.m_Position.x == Approx(0.f));
		CHECK(first.m_Position.y == Approx(0.f));
		CHECK(first.m_Position.z == Approx(0.f));
		CHECK(first.m_Normal.x == Approx(0.f));
		CHECK(first.m_Normal.y == Approx(0.f));
		CHECK(first.m_Normal.z == Approx(1.f));
		CHECK(first.m_TexCoord.x == Approx(0.f));
		CHECK(first.m_TexCoord.y == Approx(0.f));

		const pg::ModelVertex& third = model->GetVertices()[2];
		CHECK(third.m_Position.x == Approx(1.f));
		CHECK(third.m_Position.y == Approx(1.f));
		CHECK(third.m_TexCoord.x == Approx(1.f));
		CHECK(third.m_TexCoord.y == Approx(1.f));
	}

	// ---------------------------------------------------------------------------
	// Guard: a missing file yields an empty model rather than crashing.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Model::MissingFileYieldsEmptyModel")
	{
		pg::S_Ptr<pg::Model> model = pg::Model::Create("Assets/UT/Models/does_not_exist.obj");
		REQUIRE(model != nullptr);
		CHECK(model->GetVertices().empty());
		CHECK(model->GetIndices().empty());
	}

} // namespace CatchTestsetFail
