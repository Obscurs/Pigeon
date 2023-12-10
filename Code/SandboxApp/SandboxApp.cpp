#include <Pigeon.h>

#include "imgui/imgui.h"

namespace
{
	// Simple vertex shader
	char* s_VsCode =
		"struct VS_INPUT\n"
		"{\n"
		"	float4 Pos : POSITION;\n"
		"	float4 Col : COLOR;\n"
		"};\n"
		"struct PS_INPUT\n"
		"{\n"
		"	float4 Pos : SV_POSITION; \n"
		"	float4 Col : COLOR; \n"
		"};\n"
		"PS_INPUT main(VS_INPUT input)\n"
		"{\n"
		"	PS_INPUT output;\n"
		"	output.Pos = input.Pos; // Pass position to rasterizer\n"
		"	output.Col = input.Col; // Pass color to pixel shader\n"
		"	return output;\n"
		"};";

	// Simple pixel shader
	char* s_PsCode =
		"struct PS_INPUT\n"
		"{\n"
		"	float4 Pos : SV_POSITION;\n"
		"	float4 Col : COLOR;\n"
		"};\n"
		"float4 main(PS_INPUT input) : SV_TARGET\n"
		"{\n"
		"    return input.Col; // Output the interpolated color\n"
		"};";


	float s_OurVertices[3 * 7] = {
		 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		 0.45f, -0.5, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		 -0.45f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f
	};

	uint32_t s_Indices[3] = { 0, 1, 2 };

	class ExampleLayer : public pigeon::Layer
	{
	public:
		ExampleLayer()
			: Layer("Example")
		{
			m_VertexBuffer.reset(pigeon::VertexBuffer::Create(s_OurVertices, sizeof(s_OurVertices)));
			m_IndexBuffer.reset(pigeon::IndexBuffer::Create(s_Indices, sizeof(s_Indices) / sizeof(uint32_t)));

			pigeon::BufferLayout buffLayout = {
				{ pigeon::ShaderDataType::Float3, "POSITION" },
				{ pigeon::ShaderDataType::Float4, "COLOR" }
			};

			m_Shader.reset(pigeon::Shader::Create(s_VsCode, s_PsCode, buffLayout));
		}

		~ExampleLayer()
		{
			m_Shader.reset();

			m_VertexBuffer.reset();
			m_IndexBuffer.reset();
		}

		void OnUpdate() override
		{
			pigeon::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.f });

			pigeon::Renderer::BeginScene();

			m_VertexBuffer->Bind();
			m_IndexBuffer->Bind();
			m_Shader->Bind();

			pigeon::Renderer::Submit();
			pigeon::Renderer::EndScene();

			//if (pigeon::Input::IsKeyPressed(PG_KEY_TAB))
			//	PG_TRACE("Tab key is pressed (poll)!");
		}

		virtual void OnImGuiRender() override
		{
			ImGui::Begin("Test");
			ImGui::Text("Hello World");
			ImGui::End();
		}

		void OnEvent(pigeon::Event& event) override
		{
			/*if (event.GetEventType() == pigeon::EventType::KeyPressed)
			{
				pigeon::KeyPressedEvent& e = (pigeon::KeyPressedEvent&)event;
				if (e.GetKeyCode() == PG_KEY_TAB)
					PG_TRACE("Tab key is pressed (event)!");
				PG_TRACE("{0}", (char)e.GetKeyCode());
			}*/
		}

	private:
		std::unique_ptr<pigeon::VertexBuffer> m_VertexBuffer;
		std::unique_ptr<pigeon::IndexBuffer> m_IndexBuffer;
		std::unique_ptr<pigeon::Shader> m_Shader;
	};
}

class Sandbox : public pigeon::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox()
	{

	}
};

pigeon::Application* pigeon::CreateApplication()
{
	return new Sandbox();
}