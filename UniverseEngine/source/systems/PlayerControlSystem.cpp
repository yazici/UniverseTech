#include "PlayerControlSystem.h"
#include "../UniEngine.h"
#include "../UniInput.h"

void PlayerControlSystem::receive(ECS::World* world, const InputEvent& event) {
	//std::cout << "Got an input event!" << std::endl;

	auto input = UniEngine::GetInstance().GetInputManager();

	switch (event.axis)
	{
	case UniInput::AxisThrust:
		m_InputDirection.z = event.value;
		break;
	case UniInput::AxisStrafe:
		std::cout << "Strafing: " << event.value << std::endl;
		m_InputDirection.x = event.value;
		break;
	case UniInput::AxisAscend:
		std::cout << "Ascending: " << event.value << std::endl;
		m_InputDirection.y = event.value;
		break;
	case UniInput::AxisPitch:
		m_InputRotation.x = event.value;
		break;
	case UniInput::AxisYaw:
		m_InputRotation.y = -event.value;
		break;
	case UniInput::ButtonRightClick:
		isFullStop = event.value == 1.f;
		break;
	case UniInput::ButtonBoostUp:
		if(m_BoostFactor < 1024.f)
			m_BoostFactor *= 2.0f;
		break;
	case UniInput::ButtonBoostDown:
		if(m_BoostFactor > 1.f / 1024.f)
			m_BoostFactor /= 2.0f;
		break;
	case UniInput::ButtonRollLeft:
		m_InputRotation.z = event.value;
		break;
	case UniInput::ButtonRollRight:
		m_InputRotation.z = -event.value;
		break;

	}

}

void PlayerControlSystem::tick(ECS::World* world, float deltaTime) {
	world->each<PlayerControlComponent>([&](ECS::Entity* entity, ECS::ComponentHandle<PlayerControlComponent> player) {


		auto movement = entity->get<MovementComponent>();
		glm::vec3 direction = m_InputDirection;
		
		if(!player->HasTarget()) {
			glm::vec3 rotation = m_InputRotation;

			movement->SetBoost(m_BoostFactor);

			if(!isFullStop) {
				movement->ApplyTorque(rotation, deltaTime);
				movement->ApplyAcceleration(direction, deltaTime);
			} else {
				movement->CrashStop();
			}
		} else {
			if(!isFullStop) {
				movement->SetTarget(player->GetTargetPos());
				movement->ApplyAcceleration(direction, deltaTime);
			} else {
				movement->CrashStop();
			}
		}
	});



}
