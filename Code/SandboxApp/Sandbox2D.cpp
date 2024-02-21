#include "Sandbox2D.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui.h"

#include "Pigeon/Renderer/Renderer2D.h"
#include "Pigeon/Renderer/Sprite.h"

sbx::Sandbox2D::Sandbox2D(): pig::Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f)
{
	m_ColorQuad1 = glm::vec3(0.f, 1.f, 0.0f);
	m_PosQuad1 = glm::vec3(0.01f, 0.01f, 0.0f);
	m_ScaleQuad1 = glm::vec3(1.f, 1.f, 1.f);

	m_ColorQuad2 = glm::vec3(0.f, 1.f, 1.f);
	m_PosQuad2 = glm::vec3(1.f, 1.f, 0.f);
	m_ScaleQuad2 = glm::vec3(0.5f, 1.0f, 1.f);

	pig::Renderer2D::AddTexture("Assets/Textures/Checkerboard.png", "Checkerboard");
	
}

void sbx::Sandbox2D::OnUpdate(pig::Timestep ts)
{
	m_CameraController.OnUpdate(ts);

	pig::Renderer2D::Clear({ 0.3f, 0.3f, 0.3f, 1.f });
	pig::Renderer2D::BeginScene(m_CameraController);

	pig::Renderer2D::DrawQuad(m_PosQuad1, m_ScaleQuad1, m_ColorQuad1);
	//pig::Renderer2D::DrawQuad(m_PosQuad2, m_ScaleQuad2, "Checkerboard");

	const pig::Texture2D* texture = pig::Renderer2D::GetTexture("Checkerboard");
	const glm::vec4 texCoordsRect = texture->GetTexCoordsRect(glm::vec2(256, 640), glm::vec2(128, 256));
	pig::Sprite sprite(m_PosQuad2, m_ScaleQuad2, texCoordsRect, "Checkerboard");
	pig::Renderer2D::DrawSprite(sprite);

	pig::Renderer2D::EndScene();
}

void sbx::Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Settings");
	ImGui::ColorEdit3("Quad2 Color", glm::value_ptr(m_ColorQuad2));
	ImGui::InputFloat3("Quad2 Position", glm::value_ptr(m_PosQuad2));
	ImGui::InputFloat3("Quad2 Scale", glm::value_ptr(m_ScaleQuad2));

	ImGui::End();
}

bool sbx::Sandbox2D::OnEvent(const pig::Event& event)
{
	return m_CameraController.OnEvent(event);
}
