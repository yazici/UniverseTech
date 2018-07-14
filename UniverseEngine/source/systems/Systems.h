#pragma once

#include "../ECS.h"
#include "../components/Components.h"

class MovementSystem : public ECS::EntitySystem {
public:
	MovementSystem() {
	}

	virtual ~MovementSystem() {}

	virtual void tick(ECS::World* world, float deltaTime) override;

};

class CameraSystem : public ECS::EntitySystem {
public:
	CameraSystem() {
	}

	virtual ~CameraSystem() {}

	virtual void tick(ECS::World* world, float deltaTime) override;
};