#include "pch.h"
#include "ImGuiLayer.h"

#include <imgui.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>

#include "Pigeon/Core/Application.h"
#include "Pigeon/Core/KeyCodes.h"

#include "Platform/DirectX11/Dx11Context.h"
#include "Platform/Windows/WindowsWindow.h"

static bool s_show_demo_window = true;
static bool s_show_another_window = false;

pig::ImGuiLayer::ImGuiLayer()
	: Layer("ImGuiLayer"),
	m_Attached(false)
{
}

pig::ImGuiLayer::~ImGuiLayer()
{

}

void pig::ImGuiLayer::OnAttach()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	const float fontSize = 18.0f;// *2.0f;
	io.Fonts->AddFontFromFileTTF("Assets/Engine/Fonts/opensans/OpenSans-Bold.ttf", fontSize);
	io.FontDefault = io.Fonts->AddFontFromFileTTF("Assets/Engine/Fonts/opensans/OpenSans-Regular.ttf", fontSize);

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	auto window = static_cast<HWND>(Application::Get().GetWindow().GetNativeWindow());
	ImGui_ImplWin32_Init(window);

	auto context = static_cast<Dx11Context*>(Application::Get().GetWindow().GetGraphicsContext());

	ImGui_ImplDX11_Init(context->GetPd3dDevice(), context->GetPd3dDeviceContext());
	m_Attached = true;
}

void pig::ImGuiLayer::OnDetach()
{
	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	m_Attached = false;
}

void pig::ImGuiLayer::Begin()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void pig::ImGuiLayer::End()
{
	ImGuiIO& io = ImGui::GetIO();
	Application& app = Application::Get();
	io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());
		
	// Rendering
	ImGui::Render();

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}