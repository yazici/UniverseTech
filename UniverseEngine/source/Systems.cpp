#include <iostream>

#include "Systems.h"


void MovementSystem::tick(ECS::World* world, float deltaTime) {
	world->each<TransformComponent, MovementComponent>([&](ECS::Entity* ent, ECS::ComponentHandle<TransformComponent> transform, ECS::ComponentHandle<MovementComponent> movement) {
		
		transform->Rotate(movement->m_Rotation, movement->m_RotationSpeed * deltaTime);
		transform->MoveRelative(movement->m_dVelocity * (double)deltaTime);
		
	});
}
