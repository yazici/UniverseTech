#pragma once

#include <vector>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "ECS.h"
#include "Components.h"

class UniScene;

class UniSceneObject {
public:
	UniSceneObject();
	virtual ~UniSceneObject();

	std::shared_ptr<UniSceneObject> m_Parent;

	void SetScene(UniScene *scene);
	void SetEntity(ECS::Entity* ent);
	void SetParent(std::shared_ptr<UniSceneObject> parent);

	ECS::Entity* m_Entity = nullptr;

	ECS::ComponentHandle<TransformComponent> GetTransform();

	template<class T, class... _Types>
	void AddComponent(_Types&&... _Args);

private:
	UniScene * m_Scene;
};

template<class T, class... _Types>
void UniSceneObject::AddComponent(_Types&&... _Args) {
	m_Entity->assign<T>(_Args...);
}

