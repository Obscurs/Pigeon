#pragma once

namespace pig
{
	class PIGEON_API UUID
	{
	public:
		UUID();
		UUID(std::string& uuidStr);
		UUID(const char* uuidStr);

		~UUID() = default;

		static UUID Generate();
		bool IsNull() const;

		std::string ToString() const;

		bool operator==(const UUID& other) const;
		bool operator!=(const UUID& other) const;

		static const UUID s_NullUUID;
	private:
		void GenerateFromString(std::string& uuidStr);
		void GenerateRandom();

		std::array<unsigned char, 16> m_Data{};
	};
}