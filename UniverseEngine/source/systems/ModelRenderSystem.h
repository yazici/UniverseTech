#pragma once
#include <iostream>
#include "../3dmaths.h"
#include "../ECS.h"
#include "../components/Components.h"
#include "events.h"

using RemoveEvent = ECS::Events::OnComponentRemoved<ModelComponent>;

class ModelRenderSystem : public ECS::EntitySystem,
                          public ECS::EventSubscriber<RenderEvent>,
      public ECS::EventSubscriber<RemoveEvent> {
 public:

  virtual ~ModelRenderSystem() {}

  virtual void receive(ECS::World* world, const RenderEvent& event) override;
  virtual void receive(
      ECS::World* world,
      const RemoveEvent& event) override;

  virtual void configure(ECS::World* world) override {
    world->subscribe<RenderEvent>(this);
    world->subscribe<RemoveEvent>(this);
  }

  virtual void unconfigure(ECS::World* world) override {
    world->unsubscribeAll(this);
    // You may also unsubscribe from specific events with
    // world->unsubscribe<MyEvent>(this), but when unconfigure is called you
    // usually want to unsubscribe from all events.
  }

  virtual void tick(ECS::World* world, float deltaTime) override;
};
