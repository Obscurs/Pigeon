#pragma once

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

namespace pg 
{
	enum class ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:    return 4;
		case ShaderDataType::Float2:   return 4 * 2;
		case ShaderDataType::Float3:   return 4 * 3;
		case ShaderDataType::Float4:   return 4 * 4;
		case ShaderDataType::Mat3:     return 4 * 3 * 3;
		case ShaderDataType::Mat4:     return 4 * 4 * 4;
		case ShaderDataType::Int:      return 4;
		case ShaderDataType::Int2:     return 4 * 2;
		case ShaderDataType::Int3:     return 4 * 3;
		case ShaderDataType::Int4:     return 4 * 4;
		case ShaderDataType::Bool:     return 1;
		}

		PG_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	struct BufferElement
	{
		std::string m_Name;
		ShaderDataType m_Type;
		uint32_t m_Size;
		uint32_t m_Offset;
		bool m_Normalized;

		BufferElement() {}

		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
			: m_Name(name), m_Type(type), m_Size(ShaderDataTypeSize(type)), m_Offset(0), m_Normalized(normalized)
		{
		}

		uint32_t GetComponentCount() const
		{
			switch (m_Type)
			{
			case ShaderDataType::Float:   return 1;
			case ShaderDataType::Float2:  return 2;
			case ShaderDataType::Float3:  return 3;
			case ShaderDataType::Float4:  return 4;
			case ShaderDataType::Mat3:    return 3 * 3;
			case ShaderDataType::Mat4:    return 4 * 4;
			case ShaderDataType::Int:     return 1;
			case ShaderDataType::Int2:    return 2;
			case ShaderDataType::Int3:    return 3;
			case ShaderDataType::Int4:    return 4;
			case ShaderDataType::Bool:    return 1;
			}

			PG_CORE_ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}
	};

	class BufferLayout
	{
	public:
		BufferLayout() {}

		BufferLayout(const std::initializer_list<BufferElement>& elements)
			: m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}

		BufferLayout(const std::vector<BufferElement>& elements)
			: m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}

		inline uint32_t GetStride() const { return m_Stride; }
		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }
	private:
		void CalculateOffsetsAndStride()
		{
			uint32_t offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements)
			{
				element.m_Offset = offset;
				offset += element.m_Size;
				m_Stride += element.m_Size;
			}
		}
	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};

	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual const std::string& GetName() const = 0;

		static pg::S_Ptr<Shader> Create(const std::string& filepath);
		static pg::S_Ptr<Shader> Create(const std::string& name, const char* vertexSrc, const char* fragmentSrc, const char* buffLayout);

		virtual void UploadUniformInt(const std::string& name, int value) const = 0;

		virtual void UploadUniformFloat(const std::string& name, float value) const = 0;
		virtual void UploadUniformFloat2(const std::string& name, const glm::vec2& value) const = 0;
		virtual void UploadUniformFloat3(const std::string& name, const glm::vec3& value) const = 0;
		virtual void UploadUniformFloat4(const std::string& name, const glm::vec4& value) const = 0;

		virtual void UploadUniformMat3(const std::string& name, const glm::mat3& matrix) const = 0;
		virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) const = 0;

	protected:
		const BufferLayout& GetLayout() const { return m_Layout; }
		void SetLayout(const BufferLayout& layout) { m_Layout = layout; }

		BufferLayout m_Layout;
	};

	class ShaderLibrary
	{
	public:
		void Add(const std::string& name, const pg::S_Ptr<Shader>& shader);
		void Add(const pg::S_Ptr<Shader>& shader);
		pg::S_Ptr<Shader> Load(const std::string& filepath);
		pg::S_Ptr<Shader> Load(const std::string& name, const std::string& filepath);

		pg::S_Ptr<Shader> Get(const std::string& name);

		bool Exists(const std::string& name) const;
	private:
		std::unordered_map<std::string, pg::S_Ptr<Shader>> m_Shaders;
	};



}