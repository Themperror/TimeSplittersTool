#pragma once

#include "memoryreader.h"
struct TSVector
{
	float x, y, z;
};
struct TSQuat
{
	float x, y, z, w;
	void Load(Utility::MemoryReader& reader)
	{
		auto GetComponent = [](uint8_t a, uint8_t b) -> float
		{
			constexpr float NormalizeFraction = 2.0f / 65535.0f;
			const int32_t bi = (int32_t)b;
			const float added = (((float)a) + (float)(bi << 8));
			return (added * NormalizeFraction) - 1.0f;
		};

		uint8_t buf[8];
		reader.Read<uint8_t,8>(buf);

		x = GetComponent(buf[0], buf[1]);
		y = GetComponent(buf[2], buf[3]);
		z = GetComponent(buf[4], buf[5]);
		w = GetComponent(buf[6], buf[7]);

	}
};