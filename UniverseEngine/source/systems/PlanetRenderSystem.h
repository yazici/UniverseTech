#pragma once
#include "../ECS.h"

class PlanetRenderSystem : public ECS::EntitySystem{
public:
	PlanetRenderSystem();
	~PlanetRenderSystem();

	virtual void tick(ECS::World* world, float deltaTime) override;
};

