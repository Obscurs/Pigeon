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
	m_PosQuad1 = glm::vec3(0.0f, 0.0f, 0.0f);
	m_ScaleQuad1 = glm::vec3(0.1f, 0.1f, 1.f);
	m_OriginQuad1 = glm::vec3(0.5f, 0.5f, 0.f);

	m_ColorQuad2 = glm::vec3(0.f, 1.f, 1.f);
	m_PosQuad2 = glm::vec3(1.f, 1.f, 0.f);
	m_ScaleQuad2 = glm::vec3(0.5f, 1.0f, 1.f);
	m_OriginQuad2 = glm::vec3(0.5f, 0.5f, 0.f);

	m_PosText = glm::vec3(0.f, 0.f, 0.f);
	m_ScaleText = glm::vec3(0.3f, 0.3f, 1.f);
	m_ColorText = glm::vec3(1.f, 0.f, 0.f);

	m_TextureID1 = pig::Renderer2D::AddTexture("Assets/Textures/Checkerboard.png", pig::EMappedTextureType::eQuad);
	m_Font = std::make_shared<pig::Font>("Assets/Fonts/opensans/OpenSans-Regular.ttf");
}

void sbx::Sandbox2D::OnUpdate(const pig::Timestep& ts)
{
	m_CameraController.OnUpdate(ts);

	pig::Renderer2D::Clear({ 0.3f, 0.3f, 0.3f, 1.f });
	pig::Renderer2D::BeginScene(m_CameraController);

	glm::mat4 transformQuad1(1.f);
	transformQuad1 = glm::translate(transformQuad1, m_PosQuad1);
	transformQuad1 = glm::scale(transformQuad1, m_ScaleQuad1);

	glm::mat4 transformGrid(1.f);
	transformGrid = glm::translate(transformGrid, glm::vec3(-4.f, -4.f, 0.f));
	transformGrid = glm::scale(transformGrid, glm::vec3(8.f, 8.f, 0.f));

	//pig::Renderer2D::DrawQuad(transformGrid, "Checkerboard", glm::vec3(0.f, 0.f, 0.f));
	pig::Renderer2D::DrawQuad(transformQuad1, m_ColorQuad1, glm::vec3(0.f, 0.f, 0.f));

	const pig::Texture2D& texture = pig::Renderer2D::GetTexture(m_TextureID1);

	const glm::vec4 texCoordsRect = texture.GetTexCoordsRect(glm::vec2(512, 128), glm::vec2(128, 256)); //B5 C5

	glm::mat4 transformQuad2(1.f);
	transformQuad2 = glm::translate(transformQuad2, m_PosQuad2);
	transformQuad2 = glm::scale(transformQuad2, m_ScaleQuad2);
	pig::Sprite sprite(transformQuad2, texCoordsRect, m_TextureID1, m_OriginQuad2);
	pig::Renderer2D::DrawSprite(sprite);

	std::string textString("This is a fucking text\nEven with multiple lines");

	glm::mat4 stringTransform = glm::mat4(1.0f); // Identity matrix
	stringTransform = glm::translate(stringTransform, m_PosText); // Apply translation
	stringTransform = glm::scale(stringTransform, m_ScaleText); // Apply scaling

	pig::Renderer2D::DrawString(stringTransform, textString, m_Font, glm::vec4(m_ColorText, 1.f), 0.1f, 0.1f);

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

	//pig::Renderer2D::DrawString(stringClockTransform, timeString, pig::Font::GetDefault(), glm::vec4(timeTextColor, 1.f), 1.0f, 1.0f);
	
	pig::Renderer2D::EndScene();
}

void sbx::Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Settings");
	ImGui::ColorEdit3("Quad1 Color", glm::value_ptr(m_ColorQuad1));
	ImGui::InputFloat3("Quad1 Position", glm::value_ptr(m_PosQuad1));
	ImGui::InputFloat3("Quad1 Scale", glm::value_ptr(m_ScaleQuad1));
	ImGui::InputFloat3("Quad1 Origin", glm::value_ptr(m_OriginQuad1));

	ImGui::InputFloat3("Text Position", glm::value_ptr(m_PosText));
	ImGui::InputFloat3("Text Scale", glm::value_ptr(m_ScaleText));

	ImGui::End();
}

bool sbx::Sandbox2D::OnEvent(const pig::Event& event)
{
	return m_CameraController.OnEvent(event);
}
