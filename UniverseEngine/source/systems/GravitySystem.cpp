#include "GravitySystem.h"

GravitySystem::GravitySystem() {}

GravitySystem::~GravitySystem() {}

void GravitySystem::tick(ECS::World* world, float deltaTime) {

	double biggestMass = 0.0;
	glm::vec3 gravityDirection = glm::vec3(0.0);

	world->each<TransformComponent, PhysicsComponent, UniPlanet>([&](
		ECS::Entity* ent, ECS::ComponentHandle<TransformComponent> transform, ECS::ComponentHandle<PhysicsComponent> physics, ECS::ComponentHandle<UniPlanet> planet) {
		auto mass = physics->m_Mass;
		if(mass > biggestMass) {
			biggestMass = mass;
			gravityDirection = transform->GetPosition();
		}
	});

	world->each<TransformComponent, PhysicsComponent>([&](
		ECS::Entity* ent, ECS::ComponentHandle<TransformComponent> transform, ECS::ComponentHandle<PhysicsComponent> physics) {

		if(!physics->m_IsStatic) {

			auto position = transform->GetPosition();
			auto direction = glm::normalize(gravityDirection - position);


			auto distance = glm::distance(gravityDirection, position); // distance in metres

			if(distance > 1.0) {
				auto mass = physics->m_Mass;
				auto gravFactor = (mass * biggestMass) / pow(distance, 2.0);

				gravFactor *= (6.67408 * pow(10.0, -11.0));

				//std::cout << "Gravity force: " << gravFactor << "N." << std::endl;

				auto force = (direction * (float)gravFactor);

				physics->AddForce(force * deltaTime);
			}
		}
	});

}
