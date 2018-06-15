
#include "UniSceneObject.h"


UniSceneObject::UniSceneObject() {
}

UniSceneObject::~UniSceneObject() {}


void UniSceneObject::SetScene(UniScene *scene) {
	m_Scene = scene;
}

void UniSceneObject::SetEntity(ECS::Entity* ent) {
	m_Entity = ent;
	m_Entity->assign<Transform>();
}

ECS::ComponentHandle<Transform> UniSceneObject::GetTransform() {
	ECS::ComponentHandle<Transform> transform = m_Entity->get<Transform>();
	return transform;
}
