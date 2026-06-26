#pragma once

#include <cstring>

namespace sbx
{
	// The pipeline stage the image-generation demo is currently driving (ADR 0011). The demo advances
	// through these as each Diffusion Job finishes; eIdle and eDone both accept a fresh Generate trigger.
	enum class EImageGenStep
	{
		eIdle,
		eBackground, // restyling the original photo (img2img)
		eHint,       // rasterizing the OpenPose pose hint
		eComposite,  // painting the figure into the restyled background
		eDone
	};

	// The editable state + pipeline progress of the image-generation demo panel, owned by
	// ImageGenDemoSystem (like the text-gen panel's state singleton). The background style prompt is a
	// fixed char buffer so ImGui::InputTextMultiline can edit it in place without imgui_stdlib (not
	// compiled in this project). m_Consistency is how faithful the restyled background stays to the
	// original photo (1 = most faithful); the system maps it to an img2img denoise strength.
	struct ImageGenDemoStateSingletonComponent
	{
		ImageGenDemoStateSingletonComponent()
		{
			const char* defaultPrompt = "watercolor painting, soft pastel colors, golden hour light";
			std::strncpy(m_BackgroundPrompt, defaultPrompt, sizeof(m_BackgroundPrompt) - 1);
			m_BackgroundPrompt[sizeof(m_BackgroundPrompt) - 1] = '\0';
		}

		char m_BackgroundPrompt[1024] = {};
		float m_Consistency = 0.6f;

		// Composite-step diagnostics (the figure generation NaNs to flat grey with this LoRA): the LoRA
		// weight is panel-tunable to find a non-NaN strength, and the OpenPose ControlNet can be turned off
		// to test whether the figure renders from txt2img + LoRA alone (isolating a LoRA x ControlNet
		// interaction). Defaults match the normal pipeline so behaviour is unchanged until edited.
		float m_CompositeLoraWeight = 0.8f;
		bool m_CompositeUseControlNet = true;
		// Scales the skeleton silhouette mask: smaller hugs the figure, trimming the plain-background halo
		// ("cloud") around the composited character. Panel-tunable since the ideal value is visual.
		float m_CompositeMaskScale = 0.65f;

		EImageGenStep m_Step = EImageGenStep::eIdle;
		// True once the current generation step's Diffusion Job has been observed running, so the next
		// idle frame is recognised as that job finishing (rather than the gap before it launches).
		bool m_SawJobRunning = false;
	};
}
