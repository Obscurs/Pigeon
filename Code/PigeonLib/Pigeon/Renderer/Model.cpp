#include "pch.h"
#include "Pigeon/Renderer/Model.h"

#include <array>
#include <fstream>
#include <sstream>
#include <unordered_map>

namespace
{
	// Resolves a Wavefront face index to a 0-based offset. OBJ indices are 1-based; a negative
	// index counts back from the end of the most recently declared elements.
	int ResolveIndex(int objIndex, size_t count)
	{
		if (objIndex > 0)
		{
			return objIndex - 1;
		}
		if (objIndex < 0)
		{
			return static_cast<int>(count) + objIndex;
		}
		return -1;
	}

	// Parses one whitespace-separated face corner ("v", "v/vt", "v//vn", or "v/vt/vn") into its
	// resolved 0-based position/texcoord/normal offsets. Absent components resolve to -1.
	void ParseFaceCorner(const std::string& token, size_t positionCount, size_t texCoordCount, size_t normalCount, int& outPosition, int& outTexCoord, int& outNormal)
	{
		outPosition = -1;
		outTexCoord = -1;
		outNormal = -1;

		std::array<std::string, 3> parts;
		size_t partIndex = 0;
		std::stringstream tokenStream(token);
		std::string part;
		while (partIndex < parts.size() && std::getline(tokenStream, part, '/'))
		{
			parts[partIndex] = part;
			++partIndex;
		}

		if (!parts[0].empty())
		{
			outPosition = ResolveIndex(std::stoi(parts[0]), positionCount);
		}
		if (!parts[1].empty())
		{
			outTexCoord = ResolveIndex(std::stoi(parts[1]), texCoordCount);
		}
		if (!parts[2].empty())
		{
			outNormal = ResolveIndex(std::stoi(parts[2]), normalCount);
		}
	}
}

pg::S_Ptr<pg::Model> pg::Model::Create(const std::string& path)
{
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;

	std::vector<pg::ModelVertex> vertices;
	std::vector<uint32_t> indices;
	// Deduplicates identical face corners (same v/vt/vn token) into a single shared vertex.
	std::unordered_map<std::string, uint32_t> cornerToVertex;

	std::ifstream file(path);
	if (!file.is_open())
	{
		PG_CORE_WARN("Could not open model file: {0}", path);
		return std::make_shared<pg::Model>(std::move(vertices), std::move(indices));
	}

	std::string line;
	while (std::getline(file, line))
	{
		std::stringstream lineStream(line);
		std::string prefix;
		lineStream >> prefix;

		if (prefix == "v")
		{
			glm::vec3 position{ 0.f, 0.f, 0.f };
			lineStream >> position.x >> position.y >> position.z;
			positions.push_back(position);
		}
		else if (prefix == "vn")
		{
			glm::vec3 normal{ 0.f, 0.f, 0.f };
			lineStream >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		}
		else if (prefix == "vt")
		{
			glm::vec2 texCoord{ 0.f, 0.f };
			lineStream >> texCoord.x >> texCoord.y;
			texCoords.push_back(texCoord);
		}
		else if (prefix == "f")
		{
			// Collect this face's corners as shared vertex indices, then fan-triangulate.
			std::vector<uint32_t> faceVertices;
			std::string token;
			while (lineStream >> token)
			{
				std::unordered_map<std::string, uint32_t>::const_iterator existing = cornerToVertex.find(token);
				if (existing != cornerToVertex.end())
				{
					faceVertices.push_back(existing->second);
					continue;
				}

				int positionIndex = -1;
				int texCoordIndex = -1;
				int normalIndex = -1;
				ParseFaceCorner(token, positions.size(), texCoords.size(), normals.size(), positionIndex, texCoordIndex, normalIndex);

				pg::ModelVertex vertex;
				if (positionIndex >= 0 && positionIndex < static_cast<int>(positions.size()))
				{
					vertex.m_Position = positions[positionIndex];
				}
				if (normalIndex >= 0 && normalIndex < static_cast<int>(normals.size()))
				{
					vertex.m_Normal = normals[normalIndex];
				}
				if (texCoordIndex >= 0 && texCoordIndex < static_cast<int>(texCoords.size()))
				{
					vertex.m_TexCoord = texCoords[texCoordIndex];
				}

				uint32_t newIndex = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
				cornerToVertex[token] = newIndex;
				faceVertices.push_back(newIndex);
			}

			for (size_t i = 1; i + 1 < faceVertices.size(); ++i)
			{
				indices.push_back(faceVertices[0]);
				indices.push_back(faceVertices[i]);
				indices.push_back(faceVertices[i + 1]);
			}
		}
	}

	return std::make_shared<pg::Model>(std::move(vertices), std::move(indices));
}
