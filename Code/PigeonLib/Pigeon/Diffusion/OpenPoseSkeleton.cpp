#include "pch.h"
#include "Pigeon/Diffusion/OpenPoseSkeleton.h"

#include <fstream>
#include <sstream>

#include <nlohmann/json.hpp>

namespace
{
	// Pulls the first pose out of an already-parsed OpenPose document. The format wraps everything in
	// a top-level array; the first frame's first person carries the keypoints, and the canvas size is
	// declared alongside.
	pg::S_Ptr<pg::OpenPoseSkeleton> BuildFromJson(const nlohmann::json& document)
	{
		if (!document.is_array() || document.empty())
		{
			return nullptr;
		}

		const nlohmann::json& frame = document[0];
		if (!frame.contains("people") || !frame["people"].is_array() || frame["people"].empty())
		{
			return nullptr;
		}

		const nlohmann::json& person = frame["people"][0];
		if (!person.contains("pose_keypoints_2d") || !person["pose_keypoints_2d"].is_array())
		{
			return nullptr;
		}

		const nlohmann::json& flat = person["pose_keypoints_2d"];

		std::array<pg::OpenPoseKeypoint, pg::OpenPoseSkeleton::k_JointCount> keypoints{};
		const size_t available = flat.size() / 3;
		const size_t count = available < pg::OpenPoseSkeleton::k_JointCount ? available : pg::OpenPoseSkeleton::k_JointCount;
		for (size_t i = 0; i < count; ++i)
		{
			keypoints[i].m_Position.x = flat[i * 3].get<float>();
			keypoints[i].m_Position.y = flat[i * 3 + 1].get<float>();
			keypoints[i].m_Confidence = flat[i * 3 + 2].get<float>();
		}

		float canvasWidth = 0.f;
		float canvasHeight = 0.f;
		if (frame.contains("canvas_width") && frame["canvas_width"].is_number())
		{
			canvasWidth = frame["canvas_width"].get<float>();
		}
		if (frame.contains("canvas_height") && frame["canvas_height"].is_number())
		{
			canvasHeight = frame["canvas_height"].get<float>();
		}

		return std::make_shared<pg::OpenPoseSkeleton>(keypoints, canvasWidth, canvasHeight);
	}
}

pg::S_Ptr<pg::OpenPoseSkeleton> pg::OpenPoseSkeleton::CreateFromJsonString(const std::string& jsonText)
{
	nlohmann::json document = nlohmann::json::parse(jsonText, nullptr, false);
	if (document.is_discarded())
	{
		return nullptr;
	}
	return BuildFromJson(document);
}

pg::S_Ptr<pg::OpenPoseSkeleton> pg::OpenPoseSkeleton::Create(const std::string& path)
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		PG_CORE_WARN("Could not open OpenPose skeleton file: {0}", path);
		return nullptr;
	}

	std::ostringstream stream;
	stream << file.rdbuf();
	return CreateFromJsonString(stream.str());
}
