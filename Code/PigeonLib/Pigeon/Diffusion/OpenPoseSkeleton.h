#pragma once

#include <array>
#include <cstdint>
#include <string>

#include <glm/glm.hpp>

#include "Pigeon/Core/Core.h"

namespace pg
{
	// COCO-18 (OpenPose BODY_18) joint ordering. This is the fixed topology the SD OpenPose
	// ControlNet was trained on; the rasterizer's limb sequence and colour palette are keyed to it.
	enum class EOpenPoseJoint : uint8_t
	{
		Nose = 0,
		Neck,
		RShoulder,
		RElbow,
		RWrist,
		LShoulder,
		LElbow,
		LWrist,
		RHip,
		RKnee,
		RAnkle,
		LHip,
		LKnee,
		LAnkle,
		REye,
		LEye,
		REar,
		LEar
	};

	// One COCO-18 keypoint: a position in the skeleton's canvas space and a confidence. A confidence
	// of 0 marks the joint as absent, so the rasterizer skips it and any limb that touches it.
	struct OpenPoseKeypoint
	{
		glm::vec2 m_Position{ 0.f, 0.f };
		float m_Confidence = 0.f;
	};

	// Pregenerated OpenPose pose resource: the COCO-18 keypoints of a single fixed pose plus the
	// canvas they were authored against. Holds only joint positions — the OpenPose limb/colour
	// convention lives in the rasterizer (OpenPoseHint), not in this asset. Loaded from the OpenPose
	// JSON export format (a top-level array whose first element carries people[].pose_keypoints_2d as
	// flat [x, y, confidence] triples and a canvas_width/canvas_height).
	class OpenPoseSkeleton
	{
	public:
		static constexpr size_t k_JointCount = 18;

		OpenPoseSkeleton() = default;
		OpenPoseSkeleton(const std::array<OpenPoseKeypoint, k_JointCount>& keypoints, float canvasWidth, float canvasHeight)
			: m_Keypoints(keypoints), m_CanvasWidth(canvasWidth), m_CanvasHeight(canvasHeight)
		{
		}
		~OpenPoseSkeleton() = default;

		const std::array<OpenPoseKeypoint, k_JointCount>& GetKeypoints() const { return m_Keypoints; }
		float GetCanvasWidth() const { return m_CanvasWidth; }
		float GetCanvasHeight() const { return m_CanvasHeight; }

		// Reads the OpenPose JSON file at path. Returns nullptr if the file cannot be opened or parsed.
		static pg::S_Ptr<OpenPoseSkeleton> Create(const std::string& path);
		// Parses OpenPose JSON already in memory. Returns nullptr on malformed JSON.
		static pg::S_Ptr<OpenPoseSkeleton> CreateFromJsonString(const std::string& jsonText);

	private:
		std::array<OpenPoseKeypoint, k_JointCount> m_Keypoints{};
		float m_CanvasWidth = 0.f;
		float m_CanvasHeight = 0.f;
	};
}
