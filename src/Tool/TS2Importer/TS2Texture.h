#pragma once
#include <vector>
#include <array>
#include <string>

#include "memoryreader.h"

class TS2Texture
{
	uint32_t ID;
	uint32_t UNK;
	int32_t width;
	int32_t height;
	std::array<uint32_t, 256> palette; //RGBA palette
	std::vector<unsigned char> pixels;
public:
	void LoadTexture(Utility::MemoryReader& reader);
	bool ExportToPNG(const std::string& outputPath);
};