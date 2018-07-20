#pragma once

#include <vector>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "ECS.h"
#include "components/Components.h"

class UniScene;

class UniSceneObject {
public:
	UniSceneObject();
	virtual ~UniSceneObject();

	std::shared_ptr<UniSceneObject> m_Parent;

	void SetScene(UniScene *scene);
	void SetEntity(ECS::Entity* ent, glm::vec3 pos);
	void SetParent(std::shared_ptr<UniSceneObject> parent);

	ECS::Entity* m_Entity = nullptr;

	ECS::ComponentHandle<TransformComponent> GetTransform();

	template<class T, class... _Types>
	void AddComponent(_Types&&... _Args);

	template<class T>
	T GetComponent();

	std::string GetName() { return m_Name; }
	void SetName(std::string name) { m_Name = name; }

	bool IsRendered() { return m_IsRendered; }
	void SetRendered(bool render) { m_IsRendered = render; }

protected:
	UniScene * m_Scene;
	std::string m_Name = "object";
	bool m_IsRendered = false;
};

template<class T, class... _Types>
void UniSceneObject::AddComponent(_Types&&... _Args) {
	m_Entity->assign<T>(_Args...);	
}

template<class T>
T UniSceneObject::GetComponent() {
	return m_Entity->get<T>();
}

