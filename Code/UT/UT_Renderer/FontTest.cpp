#pragma once
#include <catch2/catch.hpp>

#include "Pigeon/Core/Core.h"
#include "Pigeon/Renderer/Font.h"
#include "Pigeon/Renderer/Texture.h"

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

namespace
{
	// A real font is required because wrapping decisions depend on actual glyph advances. This loads
	// the MSDF atlas, so (like the other real-font cases) it relies on global render state and must be
	// run as part of the full UT suite rather than in isolation.
	pg::S_Ptr<pg::Font> LoadTestFont()
	{
		pg::TextureData textureData;
		return std::make_shared<pg::Font>("Assets/UT/Fonts/OpenSans-Regular.ttf", textureData);
	}

	std::vector<std::string> SplitLines(const std::string& text)
	{
		std::vector<std::string> lines;
		std::string current;
		for (char c : text)
		{
			if (c == '\n')
			{
				lines.push_back(current);
				current.clear();
			}
			else
			{
				current += c;
			}
		}
		lines.push_back(current);
		return lines;
	}
}

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Happy path: a long single-spaced sentence wraps into several lines, each of
	// which fits within the box width when rendered at the fixed font size.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Font::WrapStringBreaksLongTextIntoFittingLines")
	{
		pg::S_Ptr<pg::Font> font = LoadTestFont();
		const float fontSize = 32.f;
		const float maxWidth = 200.f;
		const std::string text = "the quick brown fox jumps over the lazy dog";

		const std::string wrapped = font->WrapString(text, fontSize, 0.f, 0.f, maxWidth);

		// Only spaces become newlines, so length is preserved (reveal indices stay valid).
		REQUIRE(wrapped.size() == text.size());
		// At least one wrap occurred.
		CHECK(wrapped.find('\n') != std::string::npos);
		// Every resulting line fits the box width.
		for (const std::string& line : SplitLines(wrapped))
		{
			const float lineWidth = font->GetStringBounds(line, 0.f, 0.f).x * fontSize;
			CHECK(lineWidth <= maxWidth + 0.01f);
		}
	}

	// ---------------------------------------------------------------------------
	// Edge: the only characters that change are spaces turned into newlines; no
	// characters are added or removed and no non-space is altered.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Font::WrapStringOnlyConvertsSpacesToNewlines")
	{
		pg::S_Ptr<pg::Font> font = LoadTestFont();
		const std::string text = "the quick brown fox jumps over the lazy dog";

		const std::string wrapped = font->WrapString(text, 32.f, 0.f, 0.f, 150.f);

		REQUIRE(wrapped.size() == text.size());
		for (size_t i = 0; i < text.size(); ++i)
		{
			if (wrapped[i] != text[i])
			{
				CHECK(text[i] == ' ');
				CHECK(wrapped[i] == '\n');
			}
		}
	}

	// ---------------------------------------------------------------------------
	// Edge: a box wide enough for the whole string introduces no soft breaks.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Font::WrapStringWideBoxKeepsSingleLine")
	{
		pg::S_Ptr<pg::Font> font = LoadTestFont();
		const std::string text = "short line";

		const std::string wrapped = font->WrapString(text, 32.f, 0.f, 0.f, 100000.f);

		CHECK(wrapped == text);
		CHECK(wrapped.find('\n') == std::string::npos);
	}

	// ---------------------------------------------------------------------------
	// Edge: existing hard newlines survive wrapping untouched.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Font::WrapStringPreservesExistingNewlines")
	{
		pg::S_Ptr<pg::Font> font = LoadTestFont();
		const std::string text = "line one\nline two";

		const std::string wrapped = font->WrapString(text, 32.f, 0.f, 0.f, 100000.f);

		CHECK(wrapped == text);
	}

	// ---------------------------------------------------------------------------
	// Guard: a non-positive max width disables wrapping and returns the input.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Font::WrapStringNonPositiveWidthReturnsInput")
	{
		pg::S_Ptr<pg::Font> font = LoadTestFont();
		const std::string text = "anything at all goes here";

		CHECK(font->WrapString(text, 32.f, 0.f, 0.f, 0.f) == text);
	}
}
