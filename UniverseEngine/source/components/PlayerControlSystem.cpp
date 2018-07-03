#include "PlayerControlSystem.h"
#include "../UniEngine.h"

void PlayerControlSystem::receive(ECS::World* world, const InputEvent& event) {
	std::cout << "Got an input event!" << std::endl;

	auto input = UniEngine::GetInstance().GetInputManager();

	switch (event.axis)
	{
	case UniInput::AxisThrust:
		std::cout << "Setting THRUST: " << event.value << std::endl;
		inputDirection.z = event.value;
		break;
	case UniInput::AxisStrafe:
		inputDirection.x = event.value;
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
		glm::vec3 rotation = inputRotation;

		if(!isFullStop) {
			movement->ApplyTorque(rotation, deltaTime);
			movement->ApplyAcceleration(direction, deltaTime);
		} else {
			movement->FullStop(deltaTime);
		}
	});



}
