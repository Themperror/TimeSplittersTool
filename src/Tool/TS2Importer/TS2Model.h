#pragma once
#include <vector>
#include "memoryreader.h"

#include "TS2Mesh.h"
#include "TS2Material.h"
#include "TS2Texture.h"
#include "TS2Skeleton.h"
#include "TS2Animation.h"

#include <optional>

class TSModel
{
	std::vector<TSMesh> meshes;
	std::vector<TSMaterial> materials;
	std::vector<std::optional<TSTexture>> textures;
	std::optional<TSSkeleton> skeleton;
	std::vector<TSAnimation> animations;
public:
	void Load(Utility::MemoryReader& reader);
	void AddSkeleton(const TSSkeleton& skeleton);
	void AddAnimation(const TSAnimation& animation);

	bool ExportToGLTF(const std::string& outputPath);
};