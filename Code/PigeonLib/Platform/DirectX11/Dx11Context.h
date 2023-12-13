#pragma once

#include "Pigeon/Renderer/GraphicsContext.h"

#include <d3d11.h>

namespace pig 
{
	class Dx11Context : public GraphicsContext
	{
	public:
		struct Data
		{
			HWND m_HWnd = nullptr;
			pig::U_Ptr<ID3D11Device, pig::ReleaseDeleter> m_Pd3dDevice = nullptr;
			pig::U_Ptr<ID3D11DeviceContext, pig::ReleaseDeleter> m_Pd3dDeviceContext = nullptr;
			pig::U_Ptr<IDXGISwapChain, pig::ReleaseDeleter> m_PSwapChain = nullptr;
			UINT m_ResizeWidth = 0, m_ResizeHeight = 0;
			UINT m_Width = 0, m_Height = 0;
		};

		Dx11Context(HWND windowHandle);

		virtual void Init() override;
		virtual void Shutdown() override;
		virtual void SwapBuffers() override;
		virtual void SetSize(unsigned int width, unsigned int height) override;

#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
#endif

		bool NeedsResize() const;
		void ResizeBuffers();
		virtual unsigned int GetWidth() override { return m_Data.m_Width; }
		virtual unsigned int GetHeight() override { return m_Data.m_Height; }
		ID3D11Device* GetPd3dDevice() { return m_Data.m_Pd3dDevice.get(); }
		ID3D11DeviceContext* GetPd3dDeviceContext() { return m_Data.m_Pd3dDeviceContext.get(); }
		IDXGISwapChain* GetSwapChain() { return m_Data.m_PSwapChain.get(); }

	private:
		bool CreateDeviceD3D(HWND hWnd);
		void CleanupDeviceD3D();

		Data m_Data;
	};
}