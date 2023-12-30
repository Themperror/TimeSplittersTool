#pragma once
#include <vector>
#include <array>
#include <string>

#include "memoryreader.h"
#include "memorywriter.h"

class TSTexture
{
	uint32_t ID;
	uint32_t UNK;
	int32_t width;
	int32_t height;
	std::array<uint32_t, 256> palette; //RGBA palette
	std::vector<unsigned char> pixels;
	Utility::MemoryWriter PNGmemory;

public:
	int32_t GetWidth() const
	{
		return width;
	}
	int32_t GetHeight() const
	{
		return height;
	}
	void LoadTexture(Utility::MemoryReader& reader);
	bool ExportToPNG(const std::string& outputPath);
	bool ExportToPNGInMemory(const char*& outpngData, size_t& outSize);
};