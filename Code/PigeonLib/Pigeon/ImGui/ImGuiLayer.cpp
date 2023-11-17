#include "pch.h"
#include "ImGuiLayer.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_dx11.h"
#include "imgui/backends/imgui_impl_win32.h"

#include "Pigeon/Application.h"
#include "Pigeon/KeyCodes.h"

#include "Platform/Windows/WindowsWindow.h"

static bool s_show_demo_window = true;
static bool s_show_another_window = false;

namespace pigeon 
{
	ImGuiLayer::ImGuiLayer()
		: Layer("ImGuiLayer")
	{
	}

	ImGuiLayer::~ImGuiLayer()
	{
	}

	void ImGuiLayer::OnAttach()
	{
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		io.KeyMap[ImGuiKey_Tab] = PG_KEY_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = PG_KEY_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = PG_KEY_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = PG_KEY_UP;
		io.KeyMap[ImGuiKey_DownArrow] = PG_KEY_DOWNN;
		io.KeyMap[ImGuiKey_PageUp] = PG_KEY_PAGE_UP;
		io.KeyMap[ImGuiKey_PageDown] = PG_KEY_PAGE_DOWN;
		io.KeyMap[ImGuiKey_Home] = PG_KEY_HOME;
		io.KeyMap[ImGuiKey_End] = PG_KEY_END;
		io.KeyMap[ImGuiKey_Insert] = PG_KEY_INSERT;
		io.KeyMap[ImGuiKey_Delete] = PG_KEY_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = PG_KEY_BACKSPACE;
		io.KeyMap[ImGuiKey_Space] = PG_KEY_SPACE;
		io.KeyMap[ImGuiKey_Enter] = PG_KEY_ENTER;
		io.KeyMap[ImGuiKey_Escape] = PG_KEY_ESCAPE;
		io.KeyMap[ImGuiKey_A] = PG_KEY_A;
		io.KeyMap[ImGuiKey_C] = PG_KEY_C;
		io.KeyMap[ImGuiKey_V] = PG_KEY_V;
		io.KeyMap[ImGuiKey_X] = PG_KEY_X;
		io.KeyMap[ImGuiKey_Y] = PG_KEY_Y;
		io.KeyMap[ImGuiKey_Z] = PG_KEY_Z;

		auto window = static_cast<WindowsWindow::WindowData*>(Application::Get().GetWindow().GetNativeWindow());
		ImGui_ImplWin32_Init(window->m_HWnd);
		ImGui_ImplDX11_Init(window->m_Pd3dDevice, window->m_Pd3dDeviceContext);
	}

	void ImGuiLayer::OnDetach()
	{
		// Cleanup
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnUpdate()
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (s_show_demo_window)
			ImGui::ShowDemoWindow(&s_show_demo_window);

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &s_show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &s_show_another_window);

			ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);
			ImGuiIO& io = ImGui::GetIO();
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();
		}

		// 3. Show another simple window.
		if (s_show_another_window)
		{
			ImGui::Begin("Another Window", &s_show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				s_show_another_window = false;
			ImGui::End();
		}

		// Rendering
		ImGui::Render();

		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	void ImGuiLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseButtonPressedEvent>(PG_BIND_EVENT_FN(ImGuiLayer::OnMouseButtonPressedEvent));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(PG_BIND_EVENT_FN(ImGuiLayer::OnMouseButtonReleasedEvent));
		dispatcher.Dispatch<MouseMovedEvent>(PG_BIND_EVENT_FN(ImGuiLayer::OnMouseMovedEvent));
		dispatcher.Dispatch<MouseScrolledEvent>(PG_BIND_EVENT_FN(ImGuiLayer::OnMouseScrolledEvent));
		dispatcher.Dispatch<KeyPressedEvent>(PG_BIND_EVENT_FN(ImGuiLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<KeyTypedEvent>(PG_BIND_EVENT_FN(ImGuiLayer::OnKeyTypedEvent));
		dispatcher.Dispatch<KeyReleasedEvent>(PG_BIND_EVENT_FN(ImGuiLayer::OnKeyReleasedEvent));
		dispatcher.Dispatch<WindowResizeEvent>(PG_BIND_EVENT_FN(ImGuiLayer::OnWindowResizeEvent));
	}

	bool ImGuiLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDown[e.GetMouseButton()] = true;

		return false;
	}

	bool ImGuiLayer::OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDown[e.GetMouseButton()] = false;

		return false;
	}

	bool ImGuiLayer::OnMouseMovedEvent(MouseMovedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2(e.GetX(), e.GetY());

		return false;
	}

	bool ImGuiLayer::OnMouseScrolledEvent(MouseScrolledEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseWheelH += e.GetXOffset();
		io.MouseWheel += e.GetYOffset();

		return false;
	}

	bool ImGuiLayer::OnKeyPressedEvent(KeyPressedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[e.GetKeyCode()] = true;

		io.KeyCtrl = io.KeysDown[341] || io.KeysDown[345];
		io.KeyShift = io.KeysDown[340] || io.KeysDown[344];
		io.KeyAlt = io.KeysDown[342] || io.KeysDown[346];
		io.KeySuper = io.KeysDown[343] || io.KeysDown[347];
		return false;
	}

	bool ImGuiLayer::OnKeyReleasedEvent(KeyReleasedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[e.GetKeyCode()] = false;

		return false;
	}

	bool ImGuiLayer::OnKeyTypedEvent(KeyTypedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		int keycode = e.GetKeyCode();
		if (keycode > 0 && keycode < 0x10000)
			io.AddInputCharacter((unsigned short)keycode);

		return false;
	}

	bool ImGuiLayer::OnWindowResizeEvent(WindowResizeEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(e.GetWidth(), e.GetHeight());
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

		return false;
	}

}