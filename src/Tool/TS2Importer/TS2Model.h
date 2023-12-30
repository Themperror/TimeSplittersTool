#pragma once
#include <vector>
#include "memoryreader.h"
#include "TS2Mesh.h"
#include "TS2Material.h"

class TSModel
{
	std::vector<TSMesh> meshes;
	std::vector<TSMaterial> materials;
public:
	void Load(Utility::MemoryReader& reader);
	bool ExportToGLTF(const std::string& outputPath);
};