#pragma once

#include "Pigeon/Renderer/RendererAPI.h"

#include <d3d11.h>

namespace pig 
{
	class Dx11RendererAPI : public RendererAPI
	{
	public:
		struct Data
		{
			pig::U_Ptr<ID3D11RenderTargetView, pig::ReleaseDeleter> m_MainRenderTargetView = nullptr;

			float m_ClearColor[4] = { 1.f, 1.f, 1.f, 1.f };
			bool m_Initialized = false;
		};

#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
#endif
		virtual void SetClearColor(const glm::vec4& color) override;

		virtual void Init() override;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		virtual void Begin() override;
		virtual void End() override;
		virtual void Clear() override;

		virtual void DrawIndexed(unsigned int count) override;

	private:
		void CreateRenderTarget();
		void CleanupRenderTarget();

		Data m_Data;
	};
}