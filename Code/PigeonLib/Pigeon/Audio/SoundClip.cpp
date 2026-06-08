#include "pch.h"
#include "Pigeon/Audio/SoundClip.h"

pg::S_Ptr<pg::SoundClip> pg::SoundClip::Create(const std::string& path)
{
	return std::make_shared<pg::SoundClip>(path);
}
