
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

ECS::ComponentHandle<TransformComponent> UniSceneObject::GetTransform() {
	ECS::ComponentHandle<TransformComponent> transform = m_Entity->get<TransformComponent>();
	return transform;
}
