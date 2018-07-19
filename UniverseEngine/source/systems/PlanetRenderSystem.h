#pragma once
#include "../ECS.h"
#include "events.h"


class PlanetRenderSystem : public ECS::EntitySystem, public ECS::EventSubscriber<CameraPauseEvent>, ECS::EventSubscriber<PlanetZEvent> {
public:

	bool isCameraPaused = false;

	PlanetRenderSystem();
	~PlanetRenderSystem();

	virtual void receive(ECS::World* world, const CameraPauseEvent& event) override;
	virtual void receive(ECS::World* world, const PlanetZEvent& event) override;

	virtual void configure(ECS::World* world) override {
		world->subscribe<CameraPauseEvent>(this);
		world->subscribe<PlanetZEvent>(this);
	}

	virtual void unconfigure(ECS::World* world) override {
		world->unsubscribeAll(this);
		// You may also unsubscribe from specific events with world->unsubscribe<MyEvent>(this), but
		// when unconfigure is called you usually want to unsubscribe from all events.
	}

	virtual void tick(ECS::World* world, float deltaTime) override;
};

