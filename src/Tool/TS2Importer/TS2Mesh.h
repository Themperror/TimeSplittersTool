#pragma once
#include <vector>
#include <optional>
#include "memoryreader.h"
#include "TS2Material.h"
class TSMesh
{
public:
	struct MeshInfo
	{
		struct MeshOffset
		{
			uint32_t GetVertexCount()
			{
				return (UVOffset - VertexOffset) / sizeof(TSVertex);
			}
			uint32_t Offset;
			uint32_t VertexOffset;
			uint32_t UVOffset;
			uint32_t VertexColorOffset;
			uint32_t NormalOffset;
		};
		uint8_t isBone;
		uint8_t unk2;
		uint8_t parentID;
		uint8_t childID;
		uint8_t unk3[16];
		uint32_t MatIDOffset;
		uint32_t MatIDOffset2;
		uint32_t MatIDOffset3;
		uint32_t unk4;
		MeshOffset MeshOffset1;
		MeshOffset MeshOffset2;
		MeshOffset MeshOffsetTransparent;
		uint32_t unk5[12];
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
	std::vector<TSMaterial> mapMatInfo;


	static std::optional<TSMesh> Load(Utility::MemoryReader& reader, const MeshInfo::MeshOffset& info, uint32_t matRange, bool isMapMesh = false);
};