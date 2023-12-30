#pragma once
#include <vector>
#include "memoryreader.h"
class TSMesh
{
public:
	struct MeshInfo
	{
		uint8_t header[3];
		uint8_t ID;
		uint8_t unknown1[16];
		uint32_t MatIDOffset;
		uint8_t unknown2[12];
		uint32_t Offset;
		uint32_t VertexOffset;
		uint32_t UVOffset;
		uint32_t NormalOffset;
		uint32_t VertexColorOffset;
		uint8_t unknown3[88];
	};

	struct SubMeshData
	{
		uint16_t MatID;
		uint16_t ID;
		uint16_t VertexStart;
		uint16_t VertexCount;
	};

	struct TSVertex
	{
		float X;
		float Y;
		float Z;
		char SameStrip;
		char Flag;
		uint16_t Scale;
	};

	struct TSUV
	{
		float U;
		float V;
		float W;
	};

	std::vector<TSVertex> vertices;
	std::vector<TSUV> uvs;
	std::vector<TSVertex> normals;
	std::vector<uint32_t> vertexColors;
	std::vector<SubMeshData> submeshData;


	void Load(Utility::MemoryReader& reader, const MeshInfo& info);
};