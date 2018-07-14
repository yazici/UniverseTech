#include <iostream>

#include "Systems.h"


void MovementSystem::tick(ECS::World* world, float deltaTime) {
	world->each<TransformComponent, MovementComponent>([&](ECS::Entity* ent, ECS::ComponentHandle<TransformComponent> transform, ECS::ComponentHandle<MovementComponent> movement) {
		if(!movement->HasTarget()) {
			transform->Rotate(movement->m_Rotation * deltaTime);
		}
		else {
			transform->RotateToTarget(movement->GetTarget());
		}
		transform->MoveRelative(movement->m_dVelocity * (double)deltaTime);
		
	});
}


void CameraSystem::tick(ECS::World* world, float deltaTime) {
	world->each<TransformComponent, CameraComponent>([&](ECS::Entity* ent, ECS::ComponentHandle<TransformComponent> transform, ECS::ComponentHandle<CameraComponent> camera) {
		camera->CalculateView(transform);
	});
}
