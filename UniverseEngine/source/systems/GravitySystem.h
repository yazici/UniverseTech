#pragma once
#include "../ECS.h"
#include "events.h"
#include "../components/Components.h"

class GravitySystem : public ECS::EntitySystem {
public:
	GravitySystem();
	~GravitySystem();

	virtual void tick(ECS::World* world, float deltaTime) override;
};

