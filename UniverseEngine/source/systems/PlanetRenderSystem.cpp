#include "PlanetRenderSystem.h"
#include "../components/UniPlanet.h"
#include "../components/Transform.h"
#include "../UniEngine.h"
#include "../UniScene.h"
#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/gtx/vector_angle.hpp"


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
		auto camDistance = glm::length(camPos);
		auto storedPos = planet->CameraPos();
		auto storedDistance = glm::length(storedPos);

		float angle = glm::angle(glm::normalize(camPos), glm::normalize(storedPos));

		glm::vec3 gridLeft = planet->GetMesh()[0];
		glm::vec3 gridRight = planet->GetMesh()[planet->GridSize() -1];

		float gridAngle = glm::angle(glm::normalize(gridLeft), glm::normalize(gridRight));
		
		float thresholdAngle =  gridAngle / 100.f;

		bool shouldUpdateCamera = false;

		if(storedDistance / camDistance > 1.2 || storedDistance / camDistance < (1.0/1.2) || angle > thresholdAngle)
			shouldUpdateCamera = true;

		if(camDistance < planet->GetRadius() + planet->GetRadius() * 0.01)
			shouldUpdateCamera = true;
		
		if(!isCameraPaused && shouldUpdateCamera)
			planet->SetCameraPosition(camPos);

		auto modelMat = transform->GetModelMat();

		planet->UpdateMesh();
		planet->UpdateBuffers();
		planet->UpdateUniformBuffers(modelMat);
	});
}
