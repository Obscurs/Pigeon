#pragma once

#include <string>
#include <vector>

namespace pg
{
	// Small filesystem helpers shared by the config loader and the runtime config writer.
	// Kept free of JSON so any text file can use them.
	bool FileExists(const std::string& path);

	// Reads the whole file into a string. Asserts and returns "" when the file cannot be opened.
	std::string ReadFileToString(const std::string& path);

	// Overwrites the file with content. Asserts when the file cannot be opened for writing.
	void WriteStringToFile(const std::string& path, const std::string& content);

	// Creates dir (and any missing parents). No-op when it already exists or path is empty.
	void EnsureDirectoryExists(const std::string& dir);

	// Full paths of every file directly under dir whose name ends in extension (e.g. ".json").
	// Returns empty when dir does not exist. Not recursive.
	std::vector<std::string> ListFilesWithExtension(const std::string& dir, const std::string& extension);

	// The filename of path without its directory or extension (e.g. "a/b/x.json" -> "x").
	std::string GetFileStem(const std::string& path);
}
