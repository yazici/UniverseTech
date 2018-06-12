#include <unordered_map>
#include "Model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

Model::Model() {
	m_Position = glm::vec3(0, 0, 0);
	m_Rotation = glm::vec3(0, 0, 0);
	m_Scale = glm::vec3(1, 1, 1);
}


Model::~Model() {}

void Model::SetFilePaths(std::string modelPath, std::string texturePath) {
	m_ModelPath = modelPath;
	m_TexturePath = texturePath;
}

void Model::Load() {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, m_ModelPath.c_str())) {
		throw std::runtime_error(err);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

	for(const auto& shape : shapes) {
		for(const auto& index : shape.mesh.indices) {
			Vertex vertex = {};

			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };

			if(uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(m_Vertices.size());
				m_Vertices.push_back(vertex);
			}

			m_Indices.push_back(uniqueVertices[vertex]);
		}
	}
}

std::vector<Vertex> Model::GetVertices() {
	return m_Vertices;
}

std::vector<uint32_t> Model::GetIndices() {
	return m_Indices;
}

std::string Model::GetTexturePath() {
	return m_TexturePath;
}
