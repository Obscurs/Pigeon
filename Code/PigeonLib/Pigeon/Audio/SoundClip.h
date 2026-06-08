#pragma once

#include <string>

#include "Pigeon/Core/Core.h"

namespace pg
{
	// Loadable audio resource. Holds the resolved path of an audio asset; the audio device decodes and
	// plays it lazily. Path-only, mirroring how UI layouts are stored in the resource map.
	class SoundClip
	{
	public:
		explicit SoundClip(const std::string& path) : m_Path(path) {}
		~SoundClip() = default;

		const std::string& GetPath() const { return m_Path; }

		static pg::S_Ptr<SoundClip> Create(const std::string& path);

	private:
		std::string m_Path;
	};
}
