#include <catch2/catch.hpp>

#include <array>

#include "Pigeon/Diffusion/OpenPoseSkeleton.h"

// The OpenPose JSON export format the engine consumes: a top-level array whose first element holds
// people[].pose_keypoints_2d (flat [x, y, confidence] triples, COCO-18 order) and a canvas size.
// This is a verbatim copy of Data/Assets/App/ImageGeneration/standing_10.json so the parser is
// exercised on the real asset shape without depending on the working directory.
namespace
{
	const char* k_StandingSkeletonJson =
		"[{\"people\": [{\"pose_keypoints_2d\": ["
		"395.7101147299011, 115.57082883765293, 1.0, "
		"449.1245581880212, 182.91947493702173, 1.0, "
		"392.2265640695889, 182.91947493702173, 1.0, "
		"375.9699943214655, 278.13652631888783, 1.0, "
		"478.15414702395606, 247.94575392951572, 1.0, "
		"506.02255230645346, 182.91947493702173, 1.0, "
		"500.21663453926635, 278.13652631888783, 1.0, "
		"416.61141869177413, 263.0411401242018, 1.0, "
		"350.4239561458428, 379.1594954679409, 1.0, "
		"329.52265218396974, 554.4982120369871, 1.0, "
		"309.78253177553415, 708.9356246441603, 1.0, "
		"432.8679884398978, 396.5772487695019, 1.0, "
		"402.67721605052543, 551.0146613766749, 1.0, "
		"278.43057583272457, 661.3270989532273, 1.0, "
		"392.2265640695889, 103.95899330327916, 1.0, "
		"402.67721605052543, 99.31425908952951, 1.0, "
		"411.9666844780247, 105.1201768567164, 1.0, "
		"446.8021910811465, 91.1859742154677, 1.0"
		"]}], \"canvas_height\": 768, \"canvas_width\": 768}]";
}

TEST_CASE("Diffusion.OpenPoseSkeleton::ParsesCanvasSize")
{
	pg::S_Ptr<pg::OpenPoseSkeleton> skeleton = pg::OpenPoseSkeleton::CreateFromJsonString(k_StandingSkeletonJson);
	REQUIRE(skeleton != nullptr);

	CHECK(skeleton->GetCanvasWidth() == Approx(768.0f));
	CHECK(skeleton->GetCanvasHeight() == Approx(768.0f));
}

TEST_CASE("Diffusion.OpenPoseSkeleton::ParsesEighteenKeypoints")
{
	pg::S_Ptr<pg::OpenPoseSkeleton> skeleton = pg::OpenPoseSkeleton::CreateFromJsonString(k_StandingSkeletonJson);
	REQUIRE(skeleton != nullptr);

	const std::array<pg::OpenPoseKeypoint, pg::OpenPoseSkeleton::k_JointCount>& kp = skeleton->GetKeypoints();
	CHECK(kp.size() == 18);
}

TEST_CASE("Diffusion.OpenPoseSkeleton::ParsesNoseKeypointFirst")
{
	pg::S_Ptr<pg::OpenPoseSkeleton> skeleton = pg::OpenPoseSkeleton::CreateFromJsonString(k_StandingSkeletonJson);
	REQUIRE(skeleton != nullptr);

	const pg::OpenPoseKeypoint& nose = skeleton->GetKeypoints()[static_cast<size_t>(pg::EOpenPoseJoint::Nose)];
	CHECK(nose.m_Position.x == Approx(395.7101147f));
	CHECK(nose.m_Position.y == Approx(115.5708288f));
	CHECK(nose.m_Confidence == Approx(1.0f));
}

TEST_CASE("Diffusion.OpenPoseSkeleton::ParsesLastKeypointAsLeftEar")
{
	pg::S_Ptr<pg::OpenPoseSkeleton> skeleton = pg::OpenPoseSkeleton::CreateFromJsonString(k_StandingSkeletonJson);
	REQUIRE(skeleton != nullptr);

	const pg::OpenPoseKeypoint& leftEar = skeleton->GetKeypoints()[static_cast<size_t>(pg::EOpenPoseJoint::LEar)];
	CHECK(leftEar.m_Position.x == Approx(446.8021910f));
	CHECK(leftEar.m_Position.y == Approx(91.1859742f));
	CHECK(leftEar.m_Confidence == Approx(1.0f));
}

TEST_CASE("Diffusion.OpenPoseSkeleton::MalformedJsonReturnsNull")
{
	pg::S_Ptr<pg::OpenPoseSkeleton> skeleton = pg::OpenPoseSkeleton::CreateFromJsonString("not valid json");
	CHECK(skeleton == nullptr);
}

TEST_CASE("Diffusion.OpenPoseSkeleton::MissingKeypointsLeaveZeroConfidence")
{
	// A pose with only two keypoints supplied: the remaining COCO-18 slots stay unset (confidence 0)
	// so the rasterizer skips them rather than drawing at the origin.
	const char* partial =
		"[{\"people\": [{\"pose_keypoints_2d\": [10.0, 20.0, 1.0, 30.0, 40.0, 1.0]}], "
		"\"canvas_width\": 512, \"canvas_height\": 512}]";

	pg::S_Ptr<pg::OpenPoseSkeleton> skeleton = pg::OpenPoseSkeleton::CreateFromJsonString(partial);
	REQUIRE(skeleton != nullptr);

	CHECK(skeleton->GetKeypoints()[0].m_Confidence == Approx(1.0f));
	CHECK(skeleton->GetKeypoints()[1].m_Confidence == Approx(1.0f));
	CHECK(skeleton->GetKeypoints()[2].m_Confidence == Approx(0.0f));
	CHECK(skeleton->GetKeypoints()[static_cast<size_t>(pg::EOpenPoseJoint::LEar)].m_Confidence == Approx(0.0f));
}
