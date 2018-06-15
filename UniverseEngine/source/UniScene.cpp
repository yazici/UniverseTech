#include "UniScene.h"
#include <memory>


UniScene::UniScene() {
	m_World = ECS::World::createWorld();
}


UniScene::~UniScene() {}

void UniScene::Tick(float deltaTime) {
	m_World->tick(deltaTime);
}

std::vector<std::shared_ptr<UniModel>> UniScene::GetModels() {

	if(!m_ModelCache.empty()) {
		return m_ModelCache;
	}

	std::vector<std::shared_ptr<UniModel>> models;
	for_each(m_SceneObjects.begin(), m_SceneObjects.end(), [&models](std::shared_ptr<UniSceneObject> so) {
		auto model = std::dynamic_pointer_cast<UniModel>(so);
		if(model) {
			models.push_back(model);
		}
	});

	return models;
}

void UniScene::AddSceneObject(std::shared_ptr<UniSceneObject> so)
{
	m_SceneObjects.push_back(so);
	m_ModelCache.clear();
	m_ModelCache = GetModels();
}

