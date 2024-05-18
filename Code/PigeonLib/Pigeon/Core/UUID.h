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

		std::array<unsigned char, 16> m_Data{0};

		friend struct std::hash<UUID>;
	};
}

//Needed to be used as key in hashmaps
namespace std
{
	template<>
	struct hash<pig::UUID>
	{
		std::size_t operator()(const pig::UUID& uuid) const noexcept
		{
			std::size_t hash = 0;
			for (const auto& byte : uuid.m_Data) {
				hash ^= std::hash<unsigned char>{}(byte)+0x9e3779b9 + (hash << 6) + (hash >> 2);
			}
			return hash;
		}
	};
}