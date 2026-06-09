#include "pch.h"
#include "Pigeon/Core/FileUtils.h"

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
