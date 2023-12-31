#include "TS2Animation.h"

void TSAnimation::Load(Utility::MemoryReader& reader)
{
	isBindPose = true;

	header = reader.Read<Header>();
	reader.Skip(24); //padding
	IDs.resize(header.numIds);
	reader.Read(IDs.data(), IDs.size());
	boneTracks.resize(header.numBones);
	reader.Read(boneTracks.data(), boneTracks.size());

	for (size_t i = 0; i < boneTracks.size(); i++)
	{
		if (isBindPose && (boneTracks[i].flags != BIND_BONE_FLAG))
		{
			isBindPose = false;
			break;
		}
	}
	//
	//if (isBindPose)
	//{
	//	reader.Skip(8);
	//}
	bindPoses.resize(header.numBones);

	for (size_t i = 0; i < boneTracks.size(); i++)
	{
		BONE_FLAG flags = boneTracks[i].flags;
		if (flags == ROOT_BONE_FLAG)
		{
			keyFrames.resize(header.numIds);
			for (size_t j = 0; j < keyFrames.size(); j++)
			{
				keyFrames[i].Load(reader);
			}
		}
		else if(flags == CHILD_BONE_FLAG)
		{
			auto& track = tracks.emplace_back();
			track.Load(reader, header.numIds);
		}
		else if (flags == BIND_BONE_FLAG)
		{
			bindPoses[i].Load(reader);
		}
	}
}

void TSAnimation::KeyFrame::Load(Utility::MemoryReader& reader)
{
	x = reader.Read<float>();
	y = reader.Read<float>();
	z = reader.Read<float>();
	rotation.Load(reader);
}

void TSAnimation::Track::Load(Utility::MemoryReader& reader, uint32_t numFrames)
{
	x = reader.Read<float>();
	y = reader.Read<float>();
	z = reader.Read<float>();

	rotations.resize(numFrames);
	for (size_t i = 0; i < numFrames; i++)
	{
		rotations[i].Load(reader);
	}
}