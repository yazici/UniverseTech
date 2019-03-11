
#include "SceneObject.h"

using namespace uni::scene;
using namespace uni::components;

void SceneObject::SetScene(Scene *scene) {
	m_Scene = scene;
}

void SceneObject::SetEntity(ECS::Entity* ent, glm::vec3 pos) {
	m_Entity = ent;
	m_Entity->assign<TransformComponent>(pos);
}

void SceneObject::SetParent(std::shared_ptr<SceneObject> parent) {
	m_Parent = parent;
	auto t = GetTransform();
	t->SetParent(m_Parent);
}

ECS::ComponentHandle<TransformComponent> SceneObject::GetTransform() {

	ECS::ComponentHandle<TransformComponent> transform = m_Entity->get<TransformComponent>();
	return transform;
}
