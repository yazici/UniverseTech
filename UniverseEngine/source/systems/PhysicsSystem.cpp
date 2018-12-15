#include "PhysicsSystem.h"
#include "../3dmaths.h"

void PhysicsSystem::tick(ECS::World* world, float deltaTime) {
	std::cout << "Ticking physics system" << std::endl;
	world->each<TransformComponent, PhysicsComponent>([&](ECS::Entity* ent, ECS::ComponentHandle<TransformComponent> transform, ECS::ComponentHandle<PhysicsComponent> physics) {

		if(physics->m_IsStatic)
			return;

		auto pos = transform->GetPosition();
		//std::cout << "Got position: " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;

		auto vel = physics->m_Velocity;
		//std::cout << "Got velocity: " << vel.x << ", " << vel.y << ", " << vel.z << std::endl;

		//std::cout << "***" << std::endl;

		transform->MoveWorld(physics->m_Velocity * (double)deltaTime);

		std::cout << "Got torque: " << glm::to_string(physics->m_Torque) << std::endl;
		
		//transform->Rotate(physics->m_Torque * deltaTime);
	});
}
