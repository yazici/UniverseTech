#pragma once
#include "../ECS.h"
#include <iostream>
#include <glm/glm.hpp>
#include "PlayerMovement.h"

struct InputEvent {
	int axis;
	float value;
};

class PlayerControlSystem : public ECS::EntitySystem, public ECS::EventSubscriber<InputEvent> {

public:

	glm::vec3 inputDirection = glm::vec3(0);
	glm::vec3 inputRotation = glm::vec3(0);


	virtual ~PlayerControlSystem() {}

	virtual void receive(ECS::World* world, const InputEvent& event) override;

	virtual void configure(ECS::World* world) override {
		world->subscribe<InputEvent>(this);
	}

	virtual void unconfigure(ECS::World* world) override {
		world->unsubscribeAll(this);
		// You may also unsubscribe from specific events with world->unsubscribe<MyEvent>(this), but
		// when unconfigure is called you usually want to unsubscribe from all events.
	}

	virtual void tick(ECS::World* world, float deltaTime) override;

};
