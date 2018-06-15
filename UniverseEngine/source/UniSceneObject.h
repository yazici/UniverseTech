#pragma once

#include <vector>
#include "glm/glm.hpp"
#include "ECS.h"
#include "Components.h"

class UniScene;

class UniSceneObject {
public:
	UniSceneObject();
	virtual ~UniSceneObject();

	void SetScene(UniScene *scene);
	void SetEntity(ECS::Entity* ent);

	ECS::Entity* m_Entity = nullptr;

	ECS::ComponentHandle<Transform> GetTransform();

private:
	UniScene * m_Scene;
};

