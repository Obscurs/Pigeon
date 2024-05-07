#include "Sandbox2D.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>

#include "Pigeon/Renderer/Font.h"
#include "Pigeon/Renderer/Renderer2D.h"
#include "Pigeon/Renderer/Sprite.h"

#include "Pigeon/Core/Clock.h"

sbx::Sandbox2D::Sandbox2D(): pig::Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f)
{
	m_ColorQuad1 = glm::vec3(0.f, 1.f, 0.0f);
	m_PosQuad1 = glm::vec3(0.01f, 0.01f, 0.0f);
	m_ScaleQuad1 = glm::vec3(1.f, 1.f, 1.f);

	m_ColorQuad2 = glm::vec3(0.f, 1.f, 1.f);
	m_PosQuad2 = glm::vec3(1.f, 1.f, 0.f);
	m_ScaleQuad2 = glm::vec3(0.5f, 1.0f, 1.f);

	m_PosText = glm::vec3(0.f, 0.f, 0.f);
	m_ScaleText = glm::vec3(0.3f, 0.3f, 1.f);
	m_ColorText = glm::vec3(1.f, 0.f, 0.f);

	pig::Renderer2D::AddTexture("Assets/Textures/Checkerboard.png", "Checkerboard", pig::EMappedTextureType::eQuad);
}

void sbx::Sandbox2D::OnUpdate(const pig::Timestep& ts)
{
	m_CameraController.OnUpdate(ts);

	pig::Renderer2D::Clear({ 0.3f, 0.3f, 0.3f, 1.f });
	pig::Renderer2D::BeginScene(m_CameraController);

	pig::Renderer2D::DrawQuad(m_PosQuad1, m_ScaleQuad1, m_ColorQuad1);
	//pig::Renderer2D::DrawQuad(m_PosQuad2, m_ScaleQuad2, "Checkerboard");

	const pig::S_Ptr<pig::Texture2D> texture = pig::Renderer2D::GetTexture("Checkerboard");
	const glm::vec4 texCoordsRect = texture->GetTexCoordsRect(glm::vec2(256, 640), glm::vec2(128, 256));
	pig::Sprite sprite(m_PosQuad2, m_ScaleQuad2, texCoordsRect, "Checkerboard");
	pig::Renderer2D::DrawSprite(sprite);

	std::string textString("This is a fucking text\nEven with multiple lines");

	glm::mat4 stringTransform = glm::mat4(1.0f); // Identity matrix
	stringTransform = glm::translate(stringTransform, m_PosText); // Apply translation
	stringTransform = glm::scale(stringTransform, m_ScaleText); // Apply scaling

	pig::Renderer2D::DrawString(textString, pig::Font::GetDefault(), stringTransform, glm::vec4(m_ColorText, 1.f), 1.0f, 1.0f);

	static pig::Clock clock;
	const pig::Timestep elapsed{ clock.Elapsed() };
	const std::string timeString
	{
		std::to_string(static_cast<int>(elapsed.AsMinutes())) + ":" +
		std::to_string(static_cast<int>(elapsed.AsSeconds()) % 60) + ":" +
		std::to_string(elapsed.AsMilliseconds() % 1000)
	};
	const glm::vec3 timeTextColor
	{
		std::fmodf(elapsed.AsSeconds()*8.f, 1.f),
		std::fmodf(elapsed.AsSeconds()*2.f, 1.f),
		std::fmodf(elapsed.AsSeconds()*4.f, 1.f),
	};

	glm::mat4 stringClockTransform = glm::mat4(1.0f); // Identity matrix
	stringClockTransform = glm::translate(stringClockTransform, glm::vec3(-0.7f, 0.7f, 0.f)); // Apply translation
	stringClockTransform = glm::scale(stringClockTransform, glm::vec3(0.3f, 0.3f, 1.0f)); // Apply scaling

	pig::Renderer2D::DrawString(timeString, pig::Font::GetDefault(), stringClockTransform, glm::vec4(timeTextColor, 1.f), 1.0f, 1.0f);

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
