#include "PlayerControlSystem.h"
#include "../UniEngine.h"
#include "../UniInput.h"

void PlayerControlSystem::receive(ECS::World* world, const InputEvent& event) {
	//std::cout << "Got an input event!" << std::endl;

	auto input = UniEngine::GetInstance().GetInputManager();

	switch (event.axis)
	{
	case UniInput::AxisThrust:
		inputDirection.z = event.value;
		break;
	case UniInput::AxisStrafe:
		std::cout << "Strafing: " << event.value << std::endl;
		inputDirection.x = event.value;
		break;
	case UniInput::AxisAscend:
		std::cout << "Ascending: " << event.value << std::endl;
		inputDirection.y = event.value;
		break;
	case UniInput::AxisPitch:
		inputRotation.x = event.value;
		break;
	case UniInput::AxisYaw:
		inputRotation.y = -event.value;
		break;
	case UniInput::ButtonRightClick:
		isFullStop = event.value == 1.f;
	}

}

void PlayerControlSystem::tick(ECS::World* world, float deltaTime) {
	world->each<PlayerControlComponent>([&](ECS::Entity* entity, ECS::ComponentHandle<PlayerControlComponent> player) {


		auto movement = entity->get<MovementComponent>();
		glm::vec3 direction = inputDirection;
		
		if(!player->HasTarget()) {
			glm::vec3 rotation = inputRotation;

			if(!isFullStop) {
				movement->ApplyTorque(rotation, deltaTime);
				movement->ApplyAcceleration(direction, deltaTime);
			} else {
				movement->FullStop(deltaTime);
			}
		} else {
			if(!isFullStop) {
				movement->SetTarget(player->GetTargetPos());
				movement->ApplyAcceleration(direction, deltaTime);
			} else {
				movement->FullStop(deltaTime);
			}
		}
	});



}
