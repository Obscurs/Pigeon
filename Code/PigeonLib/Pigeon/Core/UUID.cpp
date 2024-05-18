#include "pch.h"
#include "UUID.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <random>

const pig::UUID pig::UUID::s_NullUUID{};

pig::UUID::UUID(std::string& uuidStr)
{
	GenerateFromString(uuidStr);
}

pig::UUID::UUID(const char* uuidStr)
{
	GenerateFromString(std::string(uuidStr));
}

pig::UUID::UUID()
{
	*this = s_NullUUID;
}

pig::UUID pig::UUID::Generate()
{
	pig::UUID uuid;
	uuid.GenerateRandom();
	return std::move(uuid);
}

bool pig::UUID::IsNull() const
{
	return *this == s_NullUUID;
}

std::string pig::UUID::ToString() const
{
	std::stringstream ss;
	ss << std::hex << std::setfill('0');

	for (size_t i = 0; i < m_Data.size(); ++i) 
	{
		if (i == 4 || i == 6 || i == 8 || i == 10) 
		{
			ss << '-';
		}
		ss << std::setw(2) << static_cast<int>(m_Data[i]);
	}
	return ss.str();
}

void pig::UUID::GenerateFromString(std::string& uuidStr)
{
	PG_CORE_EXCEPT(uuidStr.size() == 36, "Invalid UUID string length");

	std::stringstream ss;
	ss << std::hex;

	size_t index = 0;
	for (size_t i = 0; i < uuidStr.size(); ++i) 
	{
		if (uuidStr[i] == '-')
			continue;

		PG_CORE_EXCEPT(index < 16, "Invalid UUID string format");

		ss.clear();
		ss.str(uuidStr.substr(i, 2));
		int byte;
		ss >> byte;

		PG_CORE_EXCEPT(!ss.fail(), "Invalid UUID string format");

		m_Data[index++] = static_cast<unsigned char>(byte);
		++i;
	}

	PG_CORE_EXCEPT(index == 16, "Invalid UUID string format");
}

bool pig::UUID::operator!=(const UUID& other) const
{
	return !(*this == other);
}

bool pig::UUID::operator==(const UUID& other) const
{
	return m_Data == other.m_Data;
}

void pig::UUID::GenerateRandom()
{
	// Seed with a real random value, if available
	std::random_device rd;

	// Initialize a Mersenne Twister pseudo-random number generator
	std::mt19937 gen(rd());

	// Uniformly distributed in range [0, 255]
	std::uniform_int_distribution<> dis(0, 255);

	for (auto& byte : m_Data)
		byte = static_cast<unsigned char>(dis(gen));

	// Set the variant to RFC 4122 (specifies the layout of the UUID)
	m_Data[8] = (m_Data[8] & 0xBF) | 0x80; // 10xxxxxx
	// Set the version to 4 (randomly generated UUID)
	m_Data[6] = (m_Data[6] & 0x4F) | 0x40; // 0100xxxx
}
