#pragma once
#include "../ECS.h"
#include "../components/Components.h"


class PhysicsSystem : public ECS::EntitySystem {
public:
	PhysicsSystem() = default;
	virtual ~PhysicsSystem() = default;

	virtual void tick(ECS::World* world, float deltaTime) override;
};