#pragma once

#include <string>

namespace pg
{
	// Small filesystem helpers shared by the config loader and the runtime config writer.
	// Kept free of JSON so any text file can use them.
	bool FileExists(const std::string& path);

	// Reads the whole file into a string. Asserts and returns "" when the file cannot be opened.
	std::string ReadFileToString(const std::string& path);

	// Overwrites the file with content. Asserts when the file cannot be opened for writing.
	void WriteStringToFile(const std::string& path, const std::string& content);
}
