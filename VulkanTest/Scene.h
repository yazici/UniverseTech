#pragma once

#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include "Vertex.h"

class SceneObject;
class Model;

class Scene {
public:
	Scene();
	~Scene();

	void Init();
	void LoadModels();
	std::vector<Vertex> GetAllVertices();
	std::vector<uint32_t> GetAllIndices();
	std::vector<uint32_t> GetAllOffsets();
	std::vector<Model*> GetAllModels();

private:
	std::vector<std::shared_ptr<SceneObject>> m_SceneObjects;
};

#endif