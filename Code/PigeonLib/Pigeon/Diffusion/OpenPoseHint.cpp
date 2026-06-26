#include "pch.h"
#include "Pigeon/Diffusion/OpenPoseHint.h"

#include <algorithm>
#include <cmath>

namespace
{
	// The canonical OpenPose BODY-18 colour palette (RGB), one entry per joint; limbs reuse it by
	// limb index. Keyed to EOpenPoseJoint ordering — do not reorder.
	const std::array<glm::ivec3, 18> k_Palette = { {
		{ 255, 0, 0 }, { 255, 85, 0 }, { 255, 170, 0 }, { 255, 255, 0 }, { 170, 255, 0 },
		{ 85, 255, 0 }, { 0, 255, 0 }, { 0, 255, 85 }, { 0, 255, 170 }, { 0, 255, 255 },
		{ 0, 170, 255 }, { 0, 85, 255 }, { 0, 0, 255 }, { 85, 0, 255 }, { 170, 0, 255 },
		{ 255, 0, 255 }, { 255, 0, 170 }, { 255, 0, 85 }
	} };

	// The 17 BODY-18 limbs as pairs of joint indices (EOpenPoseJoint values), in the canonical
	// OpenPose order; limb i is drawn with k_Palette[i].
	const std::array<std::pair<int, int>, 17> k_LimbSeq = { {
		{ 1, 2 }, { 1, 5 }, { 2, 3 }, { 3, 4 }, { 5, 6 }, { 6, 7 }, { 1, 8 }, { 8, 9 }, { 9, 10 },
		{ 1, 11 }, { 11, 12 }, { 12, 13 }, { 1, 0 }, { 0, 14 }, { 14, 16 }, { 0, 15 }, { 15, 17 }
	} };

	void SetPixel(pg::HintImage& image, int x, int y, const glm::ivec3& colour)
	{
		if (x < 0 || y < 0 || x >= static_cast<int>(image.m_Width) || y >= static_cast<int>(image.m_Height))
		{
			return;
		}
		const size_t index = (static_cast<size_t>(y) * image.m_Width + x) * 3;
		image.m_Pixels[index] = static_cast<uint8_t>(colour.r);
		image.m_Pixels[index + 1] = static_cast<uint8_t>(colour.g);
		image.m_Pixels[index + 2] = static_cast<uint8_t>(colour.b);
	}

	void DrawDisc(pg::HintImage& image, const glm::vec2& centre, float radius, const glm::ivec3& colour)
	{
		const int minX = static_cast<int>(std::floor(centre.x - radius));
		const int maxX = static_cast<int>(std::ceil(centre.x + radius));
		const int minY = static_cast<int>(std::floor(centre.y - radius));
		const int maxY = static_cast<int>(std::ceil(centre.y + radius));
		const float radiusSq = radius * radius;
		for (int y = minY; y <= maxY; ++y)
		{
			for (int x = minX; x <= maxX; ++x)
			{
				const float dx = x - centre.x;
				const float dy = y - centre.y;
				if (dx * dx + dy * dy <= radiusSq)
				{
					SetPixel(image, x, y, colour);
				}
			}
		}
	}

	// Shortest squared distance from point p to segment ab.
	float DistanceSqToSegment(const glm::vec2& p, const glm::vec2& a, const glm::vec2& b)
	{
		const glm::vec2 ab = b - a;
		const float lengthSq = ab.x * ab.x + ab.y * ab.y;
		float t = 0.f;
		if (lengthSq > 0.f)
		{
			t = std::clamp(((p.x - a.x) * ab.x + (p.y - a.y) * ab.y) / lengthSq, 0.f, 1.f);
		}
		const glm::vec2 projection = a + t * ab;
		const glm::vec2 d = p - projection;
		return d.x * d.x + d.y * d.y;
	}

	void DrawStick(pg::HintImage& image, const glm::vec2& a, const glm::vec2& b, float halfWidth, const glm::ivec3& colour)
	{
		const int minX = static_cast<int>(std::floor(std::min(a.x, b.x) - halfWidth));
		const int maxX = static_cast<int>(std::ceil(std::max(a.x, b.x) + halfWidth));
		const int minY = static_cast<int>(std::floor(std::min(a.y, b.y) - halfWidth));
		const int maxY = static_cast<int>(std::ceil(std::max(a.y, b.y) + halfWidth));
		const float halfWidthSq = halfWidth * halfWidth;
		for (int y = minY; y <= maxY; ++y)
		{
			for (int x = minX; x <= maxX; ++x)
			{
				if (DistanceSqToSegment(glm::vec2(static_cast<float>(x), static_cast<float>(y)), a, b) <= halfWidthSq)
				{
					SetPixel(image, x, y, colour);
				}
			}
		}
	}
}

pg::HintImage pg::RasterizeOpenPoseHint(const std::array<pg::OpenPoseKeypoint, pg::OpenPoseSkeleton::k_JointCount>& keypoints, uint32_t width, uint32_t height)
{
	pg::HintImage image;
	image.m_Width = width;
	image.m_Height = height;
	image.m_Pixels.assign(static_cast<size_t>(width) * height * 3, 0);

	// Line/joint sizes scale with resolution: OpenPose draws ~4px features at 512.
	const float feature = std::max(1.f, std::round(static_cast<float>(std::min(width, height)) * 4.f / 512.f));

	// Limbs first, joints on top, so the coloured discs sit over the sticks (matches OpenPose).
	for (size_t i = 0; i < k_LimbSeq.size(); ++i)
	{
		const pg::OpenPoseKeypoint& a = keypoints[k_LimbSeq[i].first];
		const pg::OpenPoseKeypoint& b = keypoints[k_LimbSeq[i].second];
		if (a.m_Confidence > 0.f && b.m_Confidence > 0.f)
		{
			DrawStick(image, a.m_Position, b.m_Position, feature * 0.5f, k_Palette[i]);
		}
	}

	for (size_t j = 0; j < keypoints.size(); ++j)
	{
		if (keypoints[j].m_Confidence > 0.f)
		{
			DrawDisc(image, keypoints[j].m_Position, feature, k_Palette[j]);
		}
	}

	return image;
}

pg::Image pg::RasterizeSkeletonMask(const std::array<pg::OpenPoseKeypoint, pg::OpenPoseSkeleton::k_JointCount>& keypoints, uint32_t width, uint32_t height, float thicknessScale)
{
	// Size the strokes from the skeleton's extent so the silhouette roughly fills a human body.
	bool any = false;
	float minX = 0.f;
	float minY = 0.f;
	float maxX = 0.f;
	float maxY = 0.f;
	for (const pg::OpenPoseKeypoint& kp : keypoints)
	{
		if (kp.m_Confidence <= 0.f)
		{
			continue;
		}
		if (!any)
		{
			minX = maxX = kp.m_Position.x;
			minY = maxY = kp.m_Position.y;
			any = true;
		}
		else
		{
			minX = std::min(minX, kp.m_Position.x);
			minY = std::min(minY, kp.m_Position.y);
			maxX = std::max(maxX, kp.m_Position.x);
			maxY = std::max(maxY, kp.m_Position.y);
		}
	}
	if (!any)
	{
		return pg::Image{};
	}

	const float spanX = maxX - minX;
	const float spanY = maxY - minY;
	const float diagonal = std::sqrt(spanX * spanX + spanY * spanY);
	// Kept modest so the silhouette hugs the figure: an over-generous mask leaves a margin of the figure's
	// plain background that shows around it when composited over a scene. thicknessScale tunes this further.
	const float limbHalfWidth = std::max(static_cast<float>(std::min(width, height)) * 0.045f, diagonal * 0.08f) * thicknessScale;

	pg::Image mask;
	mask.m_Width = width;
	mask.m_Height = height;
	mask.m_Pixels.assign(static_cast<size_t>(width) * height * 3, 0);

	const glm::ivec3 white(255, 255, 255);
	for (const std::pair<int, int>& limb : k_LimbSeq)
	{
		const pg::OpenPoseKeypoint& a = keypoints[limb.first];
		const pg::OpenPoseKeypoint& b = keypoints[limb.second];
		if (a.m_Confidence > 0.f && b.m_Confidence > 0.f)
		{
			DrawStick(mask, a.m_Position, b.m_Position, limbHalfWidth, white);
		}
	}
	for (const pg::OpenPoseKeypoint& kp : keypoints)
	{
		if (kp.m_Confidence > 0.f)
		{
			DrawDisc(mask, kp.m_Position, limbHalfWidth, white);
		}
	}

	// A wider torso block (neck to mid-hip) and a head disc, so the body fills more than thin limbs.
	const pg::OpenPoseKeypoint& neck = keypoints[static_cast<size_t>(pg::EOpenPoseJoint::Neck)];
	const pg::OpenPoseKeypoint& rHip = keypoints[static_cast<size_t>(pg::EOpenPoseJoint::RHip)];
	const pg::OpenPoseKeypoint& lHip = keypoints[static_cast<size_t>(pg::EOpenPoseJoint::LHip)];
	if (neck.m_Confidence > 0.f && rHip.m_Confidence > 0.f && lHip.m_Confidence > 0.f)
	{
		const glm::vec2 midHip = (rHip.m_Position + lHip.m_Position) * 0.5f;
		DrawStick(mask, neck.m_Position, midHip, limbHalfWidth * 1.8f, white);
	}
	const pg::OpenPoseKeypoint& nose = keypoints[static_cast<size_t>(pg::EOpenPoseJoint::Nose)];
	if (nose.m_Confidence > 0.f)
	{
		DrawDisc(mask, nose.m_Position, limbHalfWidth * 1.5f, white);
	}

	return mask;
}

std::array<pg::OpenPoseKeypoint, pg::OpenPoseSkeleton::k_JointCount> pg::TransformKeypoints(const std::array<pg::OpenPoseKeypoint, pg::OpenPoseSkeleton::k_JointCount>& keypoints, const glm::mat3& transform)
{
	std::array<pg::OpenPoseKeypoint, pg::OpenPoseSkeleton::k_JointCount> result = keypoints;
	for (size_t i = 0; i < result.size(); ++i)
	{
		const glm::vec3 transformed = transform * glm::vec3(keypoints[i].m_Position, 1.f);
		result[i].m_Position = glm::vec2(transformed.x, transformed.y);
	}
	return result;
}

glm::mat3 pg::MakeCanvasToImageTransform(float canvasWidth, float canvasHeight, uint32_t imageWidth, uint32_t imageHeight)
{
	const float scaleX = canvasWidth > 0.f ? static_cast<float>(imageWidth) / canvasWidth : 1.f;
	const float scaleY = canvasHeight > 0.f ? static_cast<float>(imageHeight) / canvasHeight : 1.f;
	glm::mat3 transform(1.f);
	transform[0][0] = scaleX;
	transform[1][1] = scaleY;
	return transform;
}
