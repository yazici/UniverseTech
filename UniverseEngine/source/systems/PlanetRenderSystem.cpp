#include "PlanetRenderSystem.h"
#include "../components/UniPlanet.h"
#include "../components/Transform.h"
#include "../UniEngine.h"
#include "../UniScene.h"


PlanetRenderSystem::PlanetRenderSystem() {}


PlanetRenderSystem::~PlanetRenderSystem() {}

void PlanetRenderSystem::receive(ECS::World* world, const CameraPauseEvent& event) {
	isCameraPaused = event.value;
}

void PlanetRenderSystem::receive(ECS::World* world, const PlanetZEvent& event) {
	
	world->each<UniPlanet>([&](ECS::Entity* entity, ECS::ComponentHandle<UniPlanet> planet) {
		planet->SetZOffset(event.value);
	});
}


void PlanetRenderSystem::tick(ECS::World* world, float deltaTime) {

	world->each<UniPlanet>([&](ECS::Entity* entity, ECS::ComponentHandle<UniPlanet> planet) {
		auto cam = UniEngine::GetInstance().GetScene()->GetCameraComponent();
		auto transform = entity->get<TransformComponent>();
		
		auto camPos = transform->TransformWSToLocal(cam->GetPosition());

		if(!isCameraPaused)
			planet->SetCameraPosition(camPos);

		auto modelMat = transform->GetModelMat();

		planet->UpdateMesh();
		planet->UpdateBuffers();
		planet->UpdateUniformBuffers(modelMat);
	});
}
