
#include "UniSceneObject.h"


UniSceneObject::UniSceneObject() {
}

UniSceneObject::~UniSceneObject() {}


void UniSceneObject::SetScene(UniScene *scene) {
	m_Scene = scene;
}

void UniSceneObject::SetEntity(ECS::Entity* ent) {
	m_Entity = ent;
	m_Entity->assign<TransformComponent>();
}

void UniSceneObject::SetParent(std::shared_ptr<UniSceneObject> parent) {
	m_Parent = parent;
	auto t = GetTransform();
	t->SetParent(m_Parent);
}

ECS::ComponentHandle<TransformComponent> UniSceneObject::GetTransform() {

	ECS::ComponentHandle<TransformComponent> transform = m_Entity->get<TransformComponent>();
	return transform;
}
