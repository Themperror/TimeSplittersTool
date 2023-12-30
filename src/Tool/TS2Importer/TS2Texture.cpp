#include "TS2Texture.h"
#include "binarywriter.h"
#include <algorithm>
#include "external/lodepng.h"

void TSTexture::LoadTexture(Utility::MemoryReader& reader)
{
	ID = reader.Read<uint32_t>();
	UNK = reader.Read<uint32_t>();
	width = reader.Read<int32_t>();
	height = reader.Read<int32_t>();

	reader.Read(palette.data(), 256);
	pixels.resize(width * height);
	reader.Read(pixels.data(), pixels.size());
	//assert(reader.IsEOF());
}

uint32_t SwizzleIdx(uint32_t Idx)
{
	uint32_t swizIdx = (Idx & 231) + ((Idx & 8) << 1) + ((Idx & 16) >> 1);
	return swizIdx;
}

std::vector<uint8_t> GetConvertedPixels(int32_t width, int32_t height, const std::vector<unsigned char>& pixels, const std::array<uint32_t,256>& palette)
{
	std::vector<uint8_t> convertedPixels;
	convertedPixels.reserve(width * height * 4);
	for (size_t i = 0; i < pixels.size(); i++)
	{
		uint32_t index = SwizzleIdx(pixels[i]);
		uint32_t RGBA = palette[index];
		RGBA &= 0xFFFFFF; //mask out alpha;
		RGBA |= std::clamp((palette[index]) * 2, 0u, 255u) << 24; //OR in adjusted alpha

		convertedPixels.push_back(RGBA & 0xFF);
		convertedPixels.push_back((RGBA >> 8) & 0xFF);
		convertedPixels.push_back((RGBA >> 16) & 0xFF);
		convertedPixels.push_back((RGBA >> 24) & 0xFF);
	}

	return convertedPixels;
}

bool TSTexture::ExportToPNGInMemory(const char*& outpngData, size_t& outSize)
{
	PNGmemory.Clear();

	std::vector<uint8_t> convertedPixels = GetConvertedPixels(width, height, pixels, palette);
	std::vector<uint8_t> encoded;
	if (lodepng::encode(encoded, convertedPixels, width, height, LodePNGColorType::LCT_RGBA) != 0)
	{
		Utility::Print("Failed encoding data to PNG");
		Utility::Break();
		return false;
	}
	PNGmemory.Write(encoded.data(), encoded.size());

	outpngData = PNGmemory.GetData();
	outSize = PNGmemory.GetSize();
	return true;
}

bool TSTexture::ExportToPNG(const std::string& outputPath)
{
	Utility::BinaryWriter writer(outputPath);
	if (writer.IsGood())
	{
		std::vector<uint8_t> convertedPixels = GetConvertedPixels(width, height, pixels, palette);
		std::vector<uint8_t> encoded;
		if (lodepng::encode(encoded, convertedPixels, width, height, LodePNGColorType::LCT_RGBA) != 0)
		{
			Utility::Print("Failed encoding data to PNG");
			Utility::Break();
			return false;
		}
		writer.Write(encoded.data(), encoded.size());
	}
	return writer.IsGood();
}