#include "TS2Model.h"

#include "external/tinygltf/tiny_gltf.h"
#include <unordered_map>

void TSModel::Load(Utility::MemoryReader& reader)
{
	uint32_t materialOffset = reader.Read<uint32_t>();
	uint32_t meshOffset = reader.Read<uint32_t>();

	reader.Seek(materialOffset);

	TSMaterial material = reader.Read<TSMaterial>();
	while (material.ID != 0xFFFFFFFF)
	{
		materials.push_back(material);
		material = reader.Read<TSMaterial>();
	}


	reader.Seek(meshOffset);

	uint32_t meshCount = reader.Read<uint32_t>();
	uint32_t meshInfoChunkSize = meshCount * sizeof(TSMesh);
	uint32_t meshInfoOffset = meshOffset - meshInfoChunkSize;

	reader.Seek(meshInfoOffset);

	std::vector<TSMesh::MeshInfo> meshInfos;

	for (size_t i = 0; i < meshCount; i++)
	{
		meshInfos.emplace_back(reader.Read<TSMesh::MeshInfo>());
	}
	std::vector<TSMesh> meshes;

	for (size_t i = 0; i < meshInfos.size(); i++)
	{
		TSMesh& mesh = meshes.emplace_back();
		mesh.Load(reader, meshInfos[i]);
	}
}

struct CombinedMesh
{
	struct SubMeshData
	{
		uint16_t MatID;
		uint16_t ID;
		std::vector<uint32_t> indices;
	};

	std::vector<float> vertices_f3;
	std::vector<float> uvs_f2;
	std::vector<float> normals_f3;
	std::vector<uint32_t> color_u1;
	std::vector<SubMeshData> submeshes;
};

CombinedMesh CombineSubmeshes(const TSMesh& tsmesh, size_t vertexOffset)
{
	CombinedMesh combinedMesh;
	std::vector<uint32_t> submeshIndices;
	if (tsmesh.vertices.size())
	{
		uint32_t currentIndex = 0;
		for (size_t i = 0; i < tsmesh.submeshData.size(); i++)
		{
			const auto& submeshData = tsmesh.submeshData[i];
			submeshIndices.clear();
			bool faceDir = false;
			uint32_t end = submeshData.VertexStart + submeshData.VertexCount;
			for (size_t j = submeshData.VertexStart; j < end; j++)
			{
				const auto& vert = tsmesh.vertices[j];
				const auto& uv = tsmesh.uvs[j];

				combinedMesh.vertices_f3.push_back(vert.X);
				combinedMesh.vertices_f3.push_back(vert.Y);
				combinedMesh.vertices_f3.push_back(vert.Z);

				combinedMesh.uvs_f2.push_back(uv.U);
				combinedMesh.uvs_f2.push_back(uv.V);

				if (tsmesh.normals.size())
				{
					const auto& normal = tsmesh.normals[j];
					combinedMesh.normals_f3.push_back(normal.X);
					combinedMesh.normals_f3.push_back(normal.Y);
					combinedMesh.normals_f3.push_back(normal.Z);
				}

				uint32_t indiceOffset = currentIndex + (uint32_t)vertexOffset;
				currentIndex++;

				if (vert.Flag == 0)
				{
					if ((faceDir && vert.SameStrip == 1) || (!faceDir && vert.SameStrip == 0))
					{
						submeshIndices.push_back(indiceOffset - 2);
						submeshIndices.push_back(indiceOffset - 1);
						submeshIndices.push_back(indiceOffset);
					}
					else
					{
						submeshIndices.push_back(indiceOffset - 1);
						submeshIndices.push_back(indiceOffset - 2);
						submeshIndices.push_back(indiceOffset);
					}
				}
				else
				{
					faceDir = true;
				}

				faceDir = !faceDir;
			}

			CombinedMesh::SubMeshData& subMesh = combinedMesh.submeshes.emplace_back();
			subMesh.ID = submeshData.ID;
			subMesh.MatID = submeshData.MatID;
			subMesh.indices = submeshIndices;			
		}
	}
	return combinedMesh;
}

bool TSModel::ExportToGLTF(const std::string& outputPath)
{
	tinygltf::TinyGLTF gltf;
	tinygltf::Model model;
	
	std::vector<float> vertices;
	std::vector<float> uvs;
	std::vector<float> normals;
	std::vector<uint32_t> colors;

	std::unordered_map<uint32_t, std::vector<uint32_t>> submeshIndicesMap;

	for (auto& tsmesh : meshes)
	{
		auto& mesh = model.meshes.emplace_back();
		const CombinedMesh& combined = CombineSubmeshes(tsmesh, vertices.size() / 3);

		for (size_t j = 0; j < combined.submeshes.size(); j++)
		{
			const auto& data = combined.submeshes[j];

			auto it = submeshIndicesMap.find(data.MatID);
			if (it != submeshIndicesMap.end())
			{
				size_t originalSize = it->second.size();
				it->second.resize(originalSize + data.indices.size());
				memcpy(it->second.data() + originalSize, data.indices.data(), sizeof(uint32_t) * data.indices.size());
			}
			else
			{
				submeshIndicesMap.try_emplace(data.MatID, data.indices);
			}
		}


		//add in range of vertices
		size_t originalSizeVerts = vertices.size();
		vertices.resize(originalSizeVerts + combined.vertices_f3.size());
		memcpy(vertices.data() + originalSizeVerts, combined.vertices_f3.data(), sizeof(float) * combined.vertices_f3.size());

		size_t originalSizeUvs = uvs.size();
		uvs.resize(originalSizeUvs + combined.uvs_f2.size());
		memcpy(uvs.data() + originalSizeUvs, combined.uvs_f2.data(), sizeof(float) * combined.uvs_f2.size());

		size_t originalSizeNormals = normals.size();
		normals.resize(originalSizeNormals + combined.normals_f3.size());
		memcpy(normals.data() + originalSizeNormals, combined.normals_f3.data(), sizeof(float) * combined.normals_f3.size());

		size_t originalSizeColors = colors.size();
		colors.resize(originalSizeColors + combined.color_u1.size());
		memcpy(colors.data() + originalSizeColors, combined.color_u1.data(), sizeof(float) * combined.color_u1.size());


		std::map<std::string, int> usedAttributes;
		{
			usedAttributes.try_emplace("POSITION", (int)model.buffers.size());
			auto& accessor = model.accessors.emplace_back();
			accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
			accessor.type = TINYGLTF_TYPE_VEC3;
			accessor.bufferView = (int)model.buffers.size();
			accessor.count = vertices.size() / 3;
			accessor.name = "POSITION";

			auto& view = model.bufferViews.emplace_back();
			view.buffer = (int)model.buffers.size();
			view.byteStride = sizeof(float) * 3;
			view.byteLength = vertices.size() * sizeof(float);
			view.name = "POSITION";
			view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

			auto& buffer = model.buffers.emplace_back();
			buffer.name = "POSITION";
			buffer.data.resize(vertices.size() * sizeof(float));
			memcpy(buffer.data.data(), vertices.data(), sizeof(float) * vertices.size());
		}
		{
			usedAttributes.try_emplace("TEXCOORD_0", (int)model.buffers.size());

			auto& accessor = model.accessors.emplace_back();
			accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
			accessor.type = TINYGLTF_TYPE_VEC2;
			accessor.bufferView = (int)model.buffers.size();
			accessor.count = uvs.size() / 2;
			accessor.name = "TEXCOORD_0";

			auto& view = model.bufferViews.emplace_back();
			view.buffer = (int)model.buffers.size();
			view.byteStride = sizeof(float) * 2;
			view.byteLength = uvs.size() * sizeof(float);
			view.name = "TEXCOORD_0";
			view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

			auto& buffer = model.buffers.emplace_back();
			buffer.name = "TEXCOORD_0";
			buffer.data.resize(uvs.size() * sizeof(float));
			memcpy(buffer.data.data(), uvs.data(), sizeof(float) * uvs.size());
		}
		if(normals.size())
		{
			usedAttributes.try_emplace("NORMAL", (int)model.buffers.size());

			auto& accessor = model.accessors.emplace_back();
			accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
			accessor.type = TINYGLTF_TYPE_VEC3;
			accessor.bufferView = (int)model.buffers.size();
			accessor.count = normals.size() / 3;
			accessor.name = "NORMAL";

			auto& view = model.bufferViews.emplace_back();
			view.buffer = (int)model.buffers.size();
			view.byteStride = sizeof(float) * 3;
			view.byteLength = normals.size() * sizeof(float);
			view.name = "NORMAL";
			view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

			auto& buffer = model.buffers.emplace_back();
			buffer.name = "NORMAL";
			buffer.data.resize(normals.size() * sizeof(float));
			memcpy(buffer.data.data(), normals.data(), sizeof(float) * normals.size());
		}
		{
			usedAttributes.try_emplace("COLOR_0", (int)model.buffers.size());

			auto& accessor = model.accessors.emplace_back();
			accessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
			accessor.type = TINYGLTF_TYPE_VEC4;
			accessor.bufferView = (int)model.buffers.size();
			accessor.count = colors.size();
			accessor.name = "COLOR_0";

			auto& view = model.bufferViews.emplace_back();
			view.buffer = (int)model.buffers.size();
			view.byteStride = sizeof(uint32_t);
			view.byteLength = colors.size() * sizeof(uint32_t);
			view.name = "COLOR_0";
			view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

			auto& buffer = model.buffers.emplace_back();
			buffer.name = "COLOR_0";
			buffer.data.resize(colors.size() * sizeof(uint32_t));
			memcpy(buffer.data.data(), colors.data(), sizeof(uint32_t) * colors.size());
		}

		std::unordered_map<uint32_t, int> materialIDtoBufferMap;
		for (const auto& indices : submeshIndicesMap)
		{
			auto it = materialIDtoBufferMap.find(indices.first);
			if (it == materialIDtoBufferMap.end())
			{
				it = materialIDtoBufferMap.try_emplace(it, indices.first, (int)model.materials.size());
				auto& material = model.materials.emplace_back();
				material.name = std::to_string(indices.first);
			}
			
			//create indices buffer
			{
				auto& accessor = model.accessors.emplace_back();
				accessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
				accessor.type = TINYGLTF_TYPE_SCALAR;
				accessor.bufferView = (int)model.buffers.size();
				accessor.count = indices.second.size();
				accessor.name = "INDICES";

				auto& view = model.bufferViews.emplace_back();
				view.buffer = (int)model.buffers.size();
				view.byteStride = sizeof(uint32_t);
				view.byteLength = indices.second.size() * sizeof(uint32_t);
				view.name = "INDICES";
				view.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

				auto& buffer = model.buffers.emplace_back();
				buffer.name = "INDICES";
				buffer.data.resize(indices.second.size() * sizeof(uint32_t));
				memcpy(buffer.data.data(), indices.second.data(), sizeof(uint32_t) * indices.second.size());
			}

			auto& mesh = model.meshes.emplace_back();
			mesh.name = std::to_string(indices.first);

			auto& primitive = mesh.primitives.emplace_back();
			primitive.indices = (int)model.buffers.size();
			primitive.mode = TINYGLTF_MODE_TRIANGLES;
			primitive.attributes = usedAttributes;
			primitive.material = it->second;
		}
	}
	return gltf.WriteGltfSceneToFile(&model, outputPath, false, true,true,false);
}