#include "Scene.h"
#include "Model.h"
#include <iostream>

const std::string MODEL_PATH = "Models/chalet.obj";
const std::string TEXTURE_PATH = "Textures/chalet.jpg";

Scene::Scene() {}

Scene::~Scene() {}

void Scene::Init() {
	
	std::shared_ptr<Model> m = std::make_shared<Model>();
	m->SetFilePaths(MODEL_PATH, TEXTURE_PATH);
	m->Load();

	m_SceneObjects.push_back(m);

	std::cout << "Created scene with " << GetAllModels().size() << " models.";
}

void Scene::LoadModels() {
	for(auto model : GetAllModels()) {
		model->Load();
	}
}

std::vector<Vertex> Scene::GetAllVertices() {
	std::vector<Vertex> vertices;
	for(auto model: GetAllModels()){
		auto mVertices = model->GetVertices();
		vertices.insert(vertices.end(), mVertices.begin(), mVertices.end());
	}
	return vertices;
}

std::vector<uint32_t> Scene::GetAllIndices() {
	std::vector<uint32_t> indices;
	for(auto model: GetAllModels()) {
		auto mIndices = model->GetIndices();
		indices.insert(indices.end(), mIndices.begin(), mIndices.end());
	}
	return indices;

}

std::vector<uint32_t> Scene::GetAllOffsets() {
	std::vector<uint32_t> offsets;
	uint32_t offset = 0;
	for(auto model: GetAllModels()) {
		offsets.push_back(offset);
		offset += static_cast<uint32_t>(model->GetVertices().size());
	}
	return offsets;
}

std::vector<Model*> Scene::GetAllModels() {
	std::vector<Model*> models;
	for(auto sceneObject : m_SceneObjects) {
		//SceneObject* so = &sceneObject;
		Model* model = dynamic_cast<Model*>(sceneObject.get());
		if(model) {
			models.push_back(model);
		}
	}
	return models;
}
