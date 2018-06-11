#pragma once
#include "SceneObject.h"
#include "Vertex.h"

class Model :
	public SceneObject {
public:
	Model();
	~Model();
	void SetFilePaths(std::string modelPath, std::string texturePath);
	void Load();
	std::vector<Vertex> GetVertices();
	std::vector<uint32_t> GetIndices();
	std::string GetTexturePath();
private:
	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
	std::string m_ModelPath;
	std::string m_TexturePath;
};

