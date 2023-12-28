#pragma once
#include <string>
#include <unordered_map>

#include "memoryreader.h"
class TS2Importer
{
	struct PakFile
	{
		enum class PakVersion
		{
			Invalid,
			P4CK, // Timesplitters 2
			P5CK, // Timesplitters Future Perfect
			P8CK  // TimeSplitters 2 (GC) (and Homefront version)
		};
		struct PakEntry
		{
			std::string name;
			uint32_t offset;
			uint32_t size;
			uint32_t extra;
		};
		struct PakHeader
		{
			char magic[4];
			uint32_t dirOffset;
			uint32_t dirSize;
			uint32_t fileNameOffset;
		} pakHeader;

		std::string filename;
		std::unordered_map<std::string, PakEntry> fileEntries;

		PakVersion GetPakVersion()
		{
			if (!strncmp(pakHeader.magic, "P4CK", 4))
			{
				return PakVersion::P4CK;
			}
			if (!strncmp(pakHeader.magic, "P5CK", 4))
			{
				return PakVersion::P5CK;
			}
			if (!strncmp(pakHeader.magic, "P8CK", 4))
			{
				return PakVersion::P8CK;
			}
			return PakVersion::Invalid;
		}

		void LoadEntriesV4(Utility::MemoryReader& reader);
		void LoadEntriesV5(Utility::MemoryReader& reader);
		void LoadEntriesV8(Utility::MemoryReader& reader);

		PakEntry LoadDirEntryV4(Utility::MemoryReader& reader);
		PakEntry LoadDirEntryV5(Utility::MemoryReader& reader);
		PakEntry LoadDirEntryV8(Utility::MemoryReader& reader);
	};

public:
	bool Import(const std::string& path);
	void Export(const std::string& inputDirectory, const std::string& output);

private:
	void ExportPAK(const PakFile& pak, const std::string& inputDirectory, const std::string& output);
	std::vector<PakFile> pakFiles;
};