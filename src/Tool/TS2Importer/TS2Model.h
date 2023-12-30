#pragma once
#include <vector>
#include "memoryreader.h"
#include "TS2Mesh.h"
#include "TS2Material.h"
#include "TS2Texture.h"

#include <optional>

class TSModel
{
	std::vector<TSMesh> meshes;
	std::vector<TSMaterial> materials;
	std::vector<std::optional<TSTexture>> textures;
public:
	void Load(Utility::MemoryReader& reader);
	bool ExportToGLTF(const std::string& outputPath);
};