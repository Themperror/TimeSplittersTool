#include "TS2Mesh.h"

void TSMesh::Load(Utility::MemoryReader& reader, const MeshInfo& info)
{
	reader.Seek(info.VertexOffset);

	constexpr size_t VERT_SIZE = sizeof(TSVertex);
	size_t sizeInBytes = info.UVOffset - info.VertexOffset;
	size_t numVerts = sizeInBytes / sizeof(TSVertex);

	vertices.reserve(numVerts);
	for (size_t i = 0; i < numVerts; i++)
	{
		vertices.push_back(reader.Read<TSVertex>());
	}

	reader.Seek(info.UVOffset);
	uvs.reserve(numVerts);
	for (size_t i = 0; i < numVerts; i++)
	{
		uvs.push_back(reader.Read<TSUV>());
	}

	reader.Seek(info.VertexColorOffset);
	vertexColors.reserve(numVerts);
	for (size_t i = 0; i < numVerts; i++)
	{
		vertexColors.push_back(reader.Read<uint32_t>());
	}

	if (info.NormalOffset)
	{
		reader.Seek(info.NormalOffset);
		for (size_t i = 0; i < numVerts; i++)
		{
			normals.push_back(reader.Read<TSVertex>());
		}
	}

	reader.Seek(info.MatIDOffset);
	SubMeshData submesh = reader.Read<SubMeshData>();
	submeshData.push_back(submesh);
	uint16_t endMarker = reader.Read<uint16_t>();
	while (endMarker) //maybe not add the last submesh?
	{
		submesh = reader.Read<SubMeshData>();
		endMarker = reader.Read<uint16_t>();
		submeshData.push_back(submesh);
	}

}