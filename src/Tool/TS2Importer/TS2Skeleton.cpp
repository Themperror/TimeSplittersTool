#include "TS2Skeleton.h"

TSSkeleton TSSkeleton::GetHumanSkeleton()
{
	TSSkeleton skeleton;
	skeleton.BindPosePath = "anim\\data\\ts2\\human_19_bindpose.raw";
	skeleton.BoneMap = {
		{0,12,15},	// Root
		{1},		// Hips
		{2,4,8},	// Waist
		{3},		// Neck
		{},			// Head
		{5},		// Right Shoulder 1
		{6},		// Right Shoulder 2
		{7},		// Right Elbow
		{},			// Right Wrist
		{9},		// Left Shoulder 1
		{10},		// Left Shoulder 2
		{11},		// Left Elbow
		{},			// Left Wrist
		{13},		// Right Hip
		{14},		// Right Knee
		{},			// Right Foot
		{16},		// Left Hip
		{17},		// Left Knee
		{},			// Left Foot
	};
	skeleton.BonePaths =
	{
		"Root/Hips",
		"Root/Hips/Waist",
		"Root/Hips/Waist/Neck",
		"Root/Hips/Waist/Neck/Head",
		"Root/Hips/Waist/Right Shoulder 1",
		"Root/Hips/Waist/Right Shoulder 1/Right Shoulder 2",
		"Root/Hips/Waist/Right Shoulder 1/Right Shoulder 2/Right Elbow",
		"Root/Hips/Waist/Right Shoulder 1/Right Shoulder 2/Right Elbow/Right Wrist",
		"Root/Hips/Waist/Left Shoulder 1",
		"Root/Hips/Waist/Left Shoulder 1/Left Shoulder 2",
		"Root/Hips/Waist/Left Shoulder 1/Left Shoulder 2/Left Elbow",
		"Root/Hips/Waist/Left Shoulder 1/Left Shoulder 2/Left Elbow/Left Wrist",
		"Root/Right Hip",
		"Root/Right Hip/Right Knee",
		"Root/Right Hip/Right Knee/Right Foot",
		"Root/Left Hip",
		"Root/Left Hip/Left Knee",
		"Root/Left Hip/Left Knee/Left Foot"
	};
	skeleton.Names =
	{
		"Hips",                 // 0
		"Waist",                // 1
		"Neck",                 // 2
		"Head",                 // 3
		"Right Shoulder 1",     // 4
		"Right Shoulder 2",     // 5
		"Right Elbow",          // 6
		"Right Wrist",          // 7
		"Left Shoulder 1",      // 8
		"Left Shoulder 2",      // 9
		"Left Elbow",           // 10
		"Left Wrist",           // 11
		"Right Hip",            // 12
		"Right Knee",           // 13
		"Right Foot",           // 14
		"Left Hip",             // 15
		"Left Knee",            // 16
		"Left Foot",            // 17
	};
	return skeleton;
}