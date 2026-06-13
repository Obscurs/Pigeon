#include <catch2/catch.hpp>

#include <array>

#include <glm/glm.hpp>

#include "Pigeon/Diffusion/OpenPoseHint.h"
#include "Pigeon/Diffusion/OpenPoseSkeleton.h"

namespace
{
	using Keypoints = std::array<pg::OpenPoseKeypoint, pg::OpenPoseSkeleton::k_JointCount>;

	// Returns the RGB triple at (x, y) of a hint image.
	glm::ivec3 PixelAt(const pg::HintImage& image, uint32_t x, uint32_t y)
	{
		const size_t index = (static_cast<size_t>(y) * image.m_Width + x) * 3;
		return glm::ivec3(image.m_Pixels[index], image.m_Pixels[index + 1], image.m_Pixels[index + 2]);
	}

	Keypoints MakeEmptyKeypoints()
	{
		Keypoints kp{};
		for (pg::OpenPoseKeypoint& k : kp)
		{
			k.m_Confidence = 0.f;
		}
		return kp;
	}
}

TEST_CASE("Diffusion.OpenPoseHint::OutputHasRequestedSizeAndThreeChannels")
{
	Keypoints kp = MakeEmptyKeypoints();
	pg::HintImage image = pg::RasterizeOpenPoseHint(kp, 64, 48);

	CHECK(image.m_Width == 64);
	CHECK(image.m_Height == 48);
	CHECK(image.m_Pixels.size() == static_cast<size_t>(64) * 48 * 3);
}

TEST_CASE("Diffusion.OpenPoseHint::EmptySkeletonRastersAllBlack")
{
	Keypoints kp = MakeEmptyKeypoints();
	pg::HintImage image = pg::RasterizeOpenPoseHint(kp, 32, 32);

	bool allBlack = true;
	for (uint8_t channel : image.m_Pixels)
	{
		if (channel != 0)
		{
			allBlack = false;
			break;
		}
	}
	CHECK(allBlack);
}

TEST_CASE("Diffusion.OpenPoseHint::JointDrawnWithItsCanonicalColour")
{
	// The nose joint (index 0) uses the canonical OpenPose colour red (255, 0, 0). A solid disc is
	// drawn at its centre, so the centre pixel is exactly that colour.
	Keypoints kp = MakeEmptyKeypoints();
	kp[static_cast<size_t>(pg::EOpenPoseJoint::Nose)].m_Position = glm::vec2(16.f, 16.f);
	kp[static_cast<size_t>(pg::EOpenPoseJoint::Nose)].m_Confidence = 1.f;

	pg::HintImage image = pg::RasterizeOpenPoseHint(kp, 32, 32);

	glm::ivec3 centre = PixelAt(image, 16, 16);
	CHECK(centre == glm::ivec3(255, 0, 0));

	// A far corner stays background black.
	CHECK(PixelAt(image, 0, 0) == glm::ivec3(0, 0, 0));
}

TEST_CASE("Diffusion.OpenPoseHint::LimbDrawnBetweenTwoValidJoints")
{
	// Nose (0) and Neck (1) are both valid -> the connecting limb is drawn, so the midpoint between
	// them is non-black even though no joint disc sits there.
	Keypoints kp = MakeEmptyKeypoints();
	kp[static_cast<size_t>(pg::EOpenPoseJoint::Nose)].m_Position = glm::vec2(20.f, 10.f);
	kp[static_cast<size_t>(pg::EOpenPoseJoint::Nose)].m_Confidence = 1.f;
	kp[static_cast<size_t>(pg::EOpenPoseJoint::Neck)].m_Position = glm::vec2(20.f, 50.f);
	kp[static_cast<size_t>(pg::EOpenPoseJoint::Neck)].m_Confidence = 1.f;

	pg::HintImage image = pg::RasterizeOpenPoseHint(kp, 64, 64);

	glm::ivec3 midpoint = PixelAt(image, 20, 30);
	CHECK(midpoint != glm::ivec3(0, 0, 0));
}

TEST_CASE("Diffusion.OpenPoseHint::LimbSkippedWhenAnEndpointIsInvalid")
{
	// Only the nose is valid; the neck has zero confidence, so the Nose-Neck limb is not drawn and
	// the midpoint stays black while the nose disc is still present.
	Keypoints kp = MakeEmptyKeypoints();
	kp[static_cast<size_t>(pg::EOpenPoseJoint::Nose)].m_Position = glm::vec2(20.f, 10.f);
	kp[static_cast<size_t>(pg::EOpenPoseJoint::Nose)].m_Confidence = 1.f;
	kp[static_cast<size_t>(pg::EOpenPoseJoint::Neck)].m_Position = glm::vec2(20.f, 50.f);
	kp[static_cast<size_t>(pg::EOpenPoseJoint::Neck)].m_Confidence = 0.f;

	pg::HintImage image = pg::RasterizeOpenPoseHint(kp, 64, 64);

	CHECK(PixelAt(image, 20, 30) == glm::ivec3(0, 0, 0));
	CHECK(PixelAt(image, 20, 10) == glm::ivec3(255, 0, 0));
}

TEST_CASE("Diffusion.OpenPoseHint::IdentityTransformLeavesKeypoints")
{
	Keypoints kp = MakeEmptyKeypoints();
	kp[0].m_Position = glm::vec2(100.f, 200.f);
	kp[0].m_Confidence = 1.f;

	Keypoints out = pg::TransformKeypoints(kp, glm::mat3(1.f));

	CHECK(out[0].m_Position.x == Approx(100.f));
	CHECK(out[0].m_Position.y == Approx(200.f));
	CHECK(out[0].m_Confidence == Approx(1.f));
}

TEST_CASE("Diffusion.OpenPoseHint::CanvasToImageTransformScalesKeypoints")
{
	// A 768-canvas keypoint at (384, 768) maps to the centre-bottom of a 256x256 hint image.
	Keypoints kp = MakeEmptyKeypoints();
	kp[0].m_Position = glm::vec2(384.f, 768.f);
	kp[0].m_Confidence = 1.f;

	glm::mat3 transform = pg::MakeCanvasToImageTransform(768.f, 768.f, 256, 256);
	Keypoints out = pg::TransformKeypoints(kp, transform);

	CHECK(out[0].m_Position.x == Approx(128.f));
	CHECK(out[0].m_Position.y == Approx(256.f));
}

TEST_CASE("Diffusion.OpenPoseHint::TransformPreservesZeroConfidenceFlag")
{
	Keypoints kp = MakeEmptyKeypoints();
	kp[3].m_Position = glm::vec2(10.f, 10.f);
	kp[3].m_Confidence = 0.f;

	Keypoints out = pg::TransformKeypoints(kp, pg::MakeCanvasToImageTransform(100.f, 100.f, 50, 50));

	CHECK(out[3].m_Confidence == Approx(0.f));
}
