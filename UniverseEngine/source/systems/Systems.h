#pragma once

#include "../ECS.h"
#include "../components/Components.h"
#include "AudioSystem.h"
#include "GravitySystem.h"
#include "ModelRenderSystem.h"
#include "PhysicsSystem.h"
#include "PlanetRenderSystem.h"
#include "PlayerControlSystem.h"

class MovementSystem : public ECS::EntitySystem {
 public:
  MovementSystem() {}

  virtual ~MovementSystem() {}

  virtual void tick(ECS::World* world, float deltaTime) override;
};

class CameraSystem : public ECS::EntitySystem {
 public:
  CameraSystem() {}

  virtual ~CameraSystem() {}

  virtual void tick(ECS::World* world, float deltaTime) override;
};