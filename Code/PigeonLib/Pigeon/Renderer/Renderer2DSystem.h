#pragma once

#include "RenderCommand.h"

#include "Pigeon/Core/UUID.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/Renderer/Buffer.h"
#include "Pigeon/Renderer/OrthographicCamera.h"
#include "Pigeon/Renderer/Shader.h"
#include "Pigeon/Renderer/Sprite.h"
#include "Pigeon/Renderer/Texture.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace pig 
{
	

	class Renderer2DSystem : public pig::System
	{
	public:
		Renderer2DSystem() = default;
		~Renderer2DSystem() = default;
		void Update(const pig::Timestep& ts) override;
		pig::SystemAccessDecl DeclareAccess() const override;
	};
}