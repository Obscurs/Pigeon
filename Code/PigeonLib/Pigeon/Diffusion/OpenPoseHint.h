#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

#include "Pigeon/Diffusion/Image.h"
#include "Pigeon/Diffusion/OpenPoseSkeleton.h"

namespace pg
{
	// The rasterized OpenPose Control Hint is an ordinary RGB8 image.
	using HintImage = Image;

	// Canonical OpenPose (BODY-18) rasterization: a black background with each limb drawn as a
	// coloured stick and each joint as a coloured disc, using the standard OpenPose palette keyed to
	// EOpenPoseJoint. Joints with confidence <= 0 are skipped, as is any limb touching one. Keypoints
	// are in the output image's pixel space — use TransformKeypoints / MakeCanvasToImageTransform to
	// map a skeleton's canvas-space keypoints there first.
	HintImage RasterizeOpenPoseHint(const std::array<OpenPoseKeypoint, OpenPoseSkeleton::k_JointCount>& keypoints, uint32_t width, uint32_t height);

	// Rasterizes a filled white-on-black silhouette of the skeleton (thick limbs + joints, a wider
	// torso, and a head disc) in the output image's pixel space. Used as an img2img inpainting mask so
	// only the character's body region is regenerated, leaving the surrounding photo untouched. Returns
	// an empty image when no keypoint is valid.
	// thicknessScale (default 1) scales the silhouette's stroke width: smaller hugs the figure more tightly
	// (less of a composited figure's plain background bleeds around it), at the risk of clipping the body.
	Image RasterizeSkeletonMask(const std::array<OpenPoseKeypoint, OpenPoseSkeleton::k_JointCount>& keypoints, uint32_t width, uint32_t height, float thicknessScale = 1.0f);

	// Applies a homogeneous 2D transform (glm::mat3 acting on vec3(x, y, 1)) to every keypoint
	// position; confidence is carried through unchanged.
	std::array<OpenPoseKeypoint, OpenPoseSkeleton::k_JointCount> TransformKeypoints(const std::array<OpenPoseKeypoint, OpenPoseSkeleton::k_JointCount>& keypoints, const glm::mat3& transform);

	// Builds the transform mapping canvas-space keypoints onto an output image of the given pixel size
	// (independent x/y scale, no translation).
	glm::mat3 MakeCanvasToImageTransform(float canvasWidth, float canvasHeight, uint32_t imageWidth, uint32_t imageHeight);
}
