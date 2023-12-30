#include "TS2Mesh.h"

std::optional<TSMesh> TSMesh::Load(Utility::MemoryReader& reader, const MeshInfo::MeshOffset& info, uint32_t matRange, bool isMapMesh)
{
	if (matRange == 0) 
		return std::nullopt;
	TSMesh mesh;

	 // Mat infos
	if (isMapMesh)
	{
		uint32_t matListSize = info.VertexOffset - info.Offset;
		if (matListSize > 0)
		{
			reader.Seek(info.Offset);
			uint32_t numMats = matListSize / sizeof(MatInfo); // MatInfo.SIZE;
			mesh.mapMatInfo.resize(numMats);
			reader.Read(mesh.mapMatInfo.data(), numMats);
		}
	}

	reader.Seek(info.VertexOffset);

	size_t sizeInBytes = info.UVOffset - info.VertexOffset;
	size_t numVerts = sizeInBytes / sizeof(TSVertex);

	mesh.vertices.reserve(numVerts);
	for (size_t i = 0; i < numVerts; i++)
	{
		mesh.vertices.push_back(reader.Read<TSVertex>());
	}

	reader.Seek(info.UVOffset);
	mesh.uvs.reserve(numVerts);
	for (size_t i = 0; i < numVerts; i++)
	{
		mesh.uvs.push_back(reader.Read<TSUV>());
	}

	if (info.VertexColorOffset)
	{
		reader.Seek(info.VertexColorOffset);
		mesh.vertexColors.reserve(numVerts);
		for (size_t i = 0; i < numVerts; i++)
		{
			mesh.vertexColors.push_back(reader.Read<uint32_t>());
		}
	}

	if (info.NormalOffset)
	{
		reader.Seek(info.NormalOffset);
		mesh.normals.reserve(numVerts);
		for (size_t i = 0; i < numVerts; i++)
		{
			mesh.normals.push_back(reader.Read<TSVertex>());
		}
	}

	reader.Seek(matRange);
	SubMeshData submesh = reader.Read<SubMeshData>();
	mesh.submeshData.push_back(submesh);
	uint16_t endMarker = 0;
	while ((endMarker = reader.Read<uint16_t>()) != 0xFFFF) //maybe not add the last submesh?
	{
		submesh = reader.Read<SubMeshData>();
		if(submesh.VertexCount != 0)
			mesh.submeshData.push_back(submesh);
	}
	return mesh;
}