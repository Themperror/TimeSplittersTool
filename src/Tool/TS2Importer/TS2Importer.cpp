#include "TS2Importer.h"
#include "common.h"
#include "stringutils.h"
#include "fileutils.h"
#include "memoryreader.h"

#include <algorithm>
#include <filesystem>
bool TS2Importer::Import(const std::string& path)
{
	std::string pakPath = Utility::ReplaceChar(path, '/', '\\');;
	if (!pakPath.ends_with('\\'))
		pakPath.append("\\");

	pakPath.append("pak/");

	auto files = Utility::GetFilesInDirectory(pakPath);
	for (size_t i = files.size() - 1; i < files.size(); i--)
	{
		if (!files[i].ends_with(".pak"))
		{
			files.erase(files.begin() + i);
		}
	}

	pakFiles.reserve(files.size());
	for (size_t i = 0; i < files.size(); i++)
	{
		const auto& fileData = Utility::ReadFileToVector(files[i]);
		Utility::MemoryReader reader((char*)fileData.data(),fileData.size());
		PakFile pak;
		pak.pakHeader = reader.Read<PakFile::PakHeader>();
		switch (pak.GetPakVersion())
		{
			case PakFile::PakVersion::P4CK:
			pak.LoadEntriesV4(reader);
			break;
			case PakFile::PakVersion::P5CK:
			Utility::Print("Import for Timesplitters Future Perfect not supported yet!");
			Utility::Break();
			break;
			case PakFile::PakVersion::P8CK:
			Utility::Print("Import for Timesplitters 2 GC / Homefront not supported yet!");
			Utility::Break();
			break;
		}

		#if DEBUG
		Utility::Print("Pak File: %s\nVersion: %c%c%c%c\nNum Files: %i\n\nFiles:", files[i].c_str(), pak.pakHeader.magic[0], pak.pakHeader.magic[1], pak.pakHeader.magic[2], pak.pakHeader.magic[3], pak.fileEntries.size());
		for(const auto& it : pak.fileEntries)
		{
			Utility::Print("%s", it.first.c_str());
		}
		#endif
		pak.filename = files[i];
		pakFiles.push_back(pak);
	}
	return true;
}

void TS2Importer::Export(const std::string& inputDirectory, const std::string& output)
{
	for (const auto& pak : pakFiles)
	{
		ExportPAK(pak, inputDirectory, output);
	}
}


void TS2Importer::ExportPAK(const PakFile& pak,const std::string& inputDirectory,  const std::string& output)
{
	const auto& fileData = Utility::ReadFileToVector(pak.filename);
	for (const auto& file : pak.fileEntries)
	{
		std::filesystem::create_directories(output + "\\" + Utility::GetPathName(file.first));
		Utility::WriteFile(fileData.data() + file.second.offset, file.second.size, Utility::GetFileName(file.first));
	}
}


void TS2Importer::PakFile::LoadEntriesV4(Utility::MemoryReader& reader)
{
	constexpr size_t ENTRY_SIZE = 60;
	size_t numEntries = pakHeader.dirSize / ENTRY_SIZE;
	reader.Seek(pakHeader.dirOffset, Utility::MemoryReader::SeekMode::BEGIN);
	for (size_t i = 0; i < numEntries; i++)
	{
		const PakEntry& entry = LoadDirEntryV4(reader);
		fileEntries.try_emplace(entry.name, entry);
	}
}

void TS2Importer::PakFile::LoadEntriesV5(Utility::MemoryReader& reader){}
void TS2Importer::PakFile::LoadEntriesV8(Utility::MemoryReader& reader){}


TS2Importer::PakFile::PakEntry TS2Importer::PakFile::LoadDirEntryV4(Utility::MemoryReader& reader)
{
	constexpr size_t FILENAME_LENGTH = 48;
	PakEntry entry;
	entry.name.resize(FILENAME_LENGTH);
	reader.Read<char, FILENAME_LENGTH>(entry.name.data());

	entry.offset = reader.Read<uint32_t>();
	entry.size = reader.Read<uint32_t>();
	entry.extra = 0; reader.Skip(sizeof(uint32_t));
	return entry;
}

TS2Importer::PakFile::PakEntry TS2Importer::PakFile::LoadDirEntryV5(Utility::MemoryReader& reader)
{
	PakEntry entry;
	uint32_t nameID = reader.Read<uint32_t>();
	entry.offset = reader.Read<uint32_t>();
	entry.size = reader.Read<uint32_t>();
	entry.extra = reader.Read<uint32_t>();

	entry.name = std::to_string(nameID);

	return entry;
}

TS2Importer::PakFile::PakEntry TS2Importer::PakFile::LoadDirEntryV8(Utility::MemoryReader& reader)
{
	PakEntry entry;
	entry.extra = reader.Read<uint32_t>();
	entry.size = reader.Read<uint32_t>();
	entry.offset = reader.Read<uint32_t>();
	//V8 has no name
	return entry;
}