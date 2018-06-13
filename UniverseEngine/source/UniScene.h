#pragma once
#include <vector>
#include <memory>

#include "UniSceneObject.h"
#include "UniModel.h"

class UniScene {
public:
	UniScene();
	~UniScene();

	std::vector<std::shared_ptr<UniSceneObject>> m_SceneObjects;
	std::vector<std::shared_ptr<UniModel>> m_ModelCache;

	std::vector<std::shared_ptr<UniModel>> GetModels();
	void AddSceneObject(std::shared_ptr<UniSceneObject> so);
};

