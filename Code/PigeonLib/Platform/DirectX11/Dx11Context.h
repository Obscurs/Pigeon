#pragma once

#include "Pigeon/Renderer/GraphicsContext.h"

#include <d3d11.h>

namespace pigeon 
{
	class Dx11Context : public GraphicsContext
	{
	public:
		Dx11Context(HWND windowHandle);

		virtual void Init() override;
		virtual void Shutdown() override;
		virtual void SwapBuffers() override;
		virtual void SetSize(unsigned int width, unsigned int height) override;

		bool NeedsResize() const;
		void ResizeBuffers();
		virtual unsigned int GetWidth() override { return m_Width; }
		virtual unsigned int GetHeight() override { return m_Height; }
		ID3D11Device* GetPd3dDevice() { return m_Pd3dDevice; }
		ID3D11DeviceContext* GetPd3dDeviceContext() { return m_Pd3dDeviceContext; }
		IDXGISwapChain* GetSwapChain() { return m_PSwapChain; }

	private:
		bool CreateDeviceD3D(HWND hWnd);
		void CleanupDeviceD3D();

		HWND m_HWnd;
		ID3D11Device* m_Pd3dDevice = nullptr;
		ID3D11DeviceContext* m_Pd3dDeviceContext = nullptr;
		IDXGISwapChain* m_PSwapChain = nullptr;
		UINT m_ResizeWidth = 0, m_ResizeHeight = 0;
		UINT m_Width = 0, m_Height = 0;
	};
}