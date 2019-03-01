#pragma once
#include <iostream>
#include "../3dmaths.h"
#include "../ECS.h"
#include "../components/Components.h"
#include "events.h"


class AudioSystem : public ECS::EntitySystem,
      public ECS::EventSubscriber<LevelStartEvent> {
 public:

  virtual ~AudioSystem() {}

  virtual void receive(ECS::World* world, const LevelStartEvent& event) override;

  virtual void configure(ECS::World* world) override {
    world->subscribe<LevelStartEvent>(this);
  }

  virtual void unconfigure(ECS::World* world) override {
    world->unsubscribeAll(this);
    // You may also unsubscribe from specific events with
    // world->unsubscribe<MyEvent>(this), but when unconfigure is called you
    // usually want to unsubscribe from all events.
  }

  virtual void tick(ECS::World* world, float deltaTime) override;
};
