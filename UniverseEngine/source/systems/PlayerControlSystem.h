#pragma once
#include "../ECS.h"
#include <iostream>
#include <glm/glm.hpp>
#include "../components/PlayerMovement.h"
#include "events.h"

class PlayerControlSystem : public ECS::EntitySystem, public ECS::EventSubscriber<InputEvent> {

public:

	glm::vec3 m_InputDirection = glm::vec3(0);
	glm::vec3 m_InputRotation = glm::vec3(0);
	float m_BoostFactor = 1.0;

	bool isCrashStop = false;
	bool isFullStop = false;


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
