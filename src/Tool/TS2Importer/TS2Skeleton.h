#pragma once
#include <vector>
#include <string>

class TSSkeleton
{
	std::vector<std::string> Names;
	std::vector<std::vector<int16_t>> BoneMap;
	std::string BindPosePath;
	std::vector<std::string> BonePaths;
public:
	enum SkeletonType
	{
		Bespoke = -2, // Model will pass in mesh section ids to be given a bone
		None = -1,
		Human = 0
	};

	static TSSkeleton GetHumanSkeleton();
};