#pragma once
#include "memoryreader.h"
#include "TS2Types.h"

#include <vector>

class TSAnimation
{
public:

	enum BONE_FLAG : uint32_t
	{
		BIND_BONE_FLAG = 0,
		CHILD_BONE_FLAG = 2,
		ROOT_BONE_FLAG = 8,
	};

	#pragma pack(push, 1) //due to misalignment on magic->version we'd incur padding otherwise
	struct Header
	{
		char magic[5];
		uint32_t version;
		char unk1[39];
		uint32_t unk2[2];
		uint32_t numIds;
		uint32_t numBones;
	};
	#pragma pack(pop)

	struct BoneTrack
	{
		uint32_t unk1;
		BONE_FLAG flags;
		float numFrames;
		uint32_t numKeyFrames;
		uint32_t unk2[4];
	};

	struct KeyFrame
	{
		float x, y, z;
		TSQuat rotation;
		void Load(Utility::MemoryReader& reader);
	};

	struct Track
	{
		float x, y, z;
		std::vector<TSQuat> rotations;

		void Load(Utility::MemoryReader& reader, uint32_t numFrames);
	};

	Header header;
	std::vector<uint32_t> IDs;
	std::vector<BoneTrack> boneTracks;
	std::vector<KeyFrame> keyFrames;
	std::vector<KeyFrame> bindPoses;
	std::vector<Track> tracks;
	std::string name;
	bool isBindPose;

	void Load(Utility::MemoryReader& reader);
};