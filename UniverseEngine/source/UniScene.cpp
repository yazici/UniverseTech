#include "UniScene.h"
#include <memory>


UniScene::UniScene() {}


UniScene::~UniScene() {}

std::vector<std::shared_ptr<UniModel>> UniScene::GetModels() {

	if(m_ModelCache.size() > 0) {
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
