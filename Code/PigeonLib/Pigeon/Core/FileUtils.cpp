#include "pch.h"
#include "Pigeon/Core/FileUtils.h"

#include <filesystem>
#include <fstream>

bool pg::FileExists(const std::string& path)
{
	std::ifstream file(path);
	return file.good();
}

std::string pg::ReadFileToString(const std::string& path)
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		PG_CORE_ASSERT(false, "Could not open file");
		return "";
	}

	std::ostringstream ss;
	ss << file.rdbuf();
	file.close();
	return ss.str();
}

void pg::WriteStringToFile(const std::string& path, const std::string& content)
{
	std::ofstream file(path, std::ios::trunc);
	if (!file.is_open())
	{
		PG_CORE_ASSERT(false, "Could not open file for writing");
		return;
	}

	file << content;
	file.close();
}

void pg::EnsureDirectoryExists(const std::string& dir)
{
	if (dir.empty())
	{
		return;
	}
	std::filesystem::create_directories(dir);
}

std::vector<std::string> pg::ListFilesWithExtension(const std::string& dir, const std::string& extension)
{
	std::vector<std::string> result;
	if (dir.empty() || !std::filesystem::exists(dir))
	{
		return result;
	}

	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(dir))
	{
		if (entry.is_regular_file() && entry.path().extension() == extension)
		{
			result.push_back(entry.path().generic_string());
		}
	}
	return result;
}

std::string pg::GetFileStem(const std::string& path)
{
	return std::filesystem::path(path).stem().generic_string();
}
