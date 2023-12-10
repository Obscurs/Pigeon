#pragma once

#include "Pigeon/Renderer/RendererAPI.h"

#include <d3d11.h>

namespace pigeon 
{
	class Dx11RendererAPI : public RendererAPI
	{
	public:
		struct Data
		{
			ID3D11RenderTargetView* m_MainRenderTargetView = nullptr;

			float m_ClearColor[4] = { 1.f, 1.f, 1.f, 1.f };
			bool m_Initialized = false;
		};

#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
#endif
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Begin() override;
		virtual void End() override;
		virtual void Clear() override;

		virtual void DrawIndexed() override;

	private:
		void CreateRenderTarget();
		void CleanupRenderTarget();

		Data m_Data;
	};
}