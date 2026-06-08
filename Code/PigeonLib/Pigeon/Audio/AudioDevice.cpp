#include "pch.h"
#include "Pigeon/Audio/AudioDevice.h"

#include "Pigeon/Renderer/Renderer.h"
#include "Pigeon/Renderer/RendererAPI.h"
#include "Platform/Miniaudio/MiniaudioAudioDevice.h"
#include "Platform/Testing/TestingAudioDevice.h"

pg::S_Ptr<pg::AudioDevice> pg::AudioDevice::Create()
{
	switch (pg::Renderer::GetAPI())
	{
		case pg::RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case pg::RendererAPI::API::DirectX11:  return std::make_shared<pg::MiniaudioAudioDevice>();
		case pg::RendererAPI::API::Testing:  return std::make_shared<pg::TestingAudioDevice>();
	}

	PG_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}
