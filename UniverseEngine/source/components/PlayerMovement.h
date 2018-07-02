#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <memory>
#include "../UniSceneObject.h"


struct PlayerControlComponent {
	PlayerControlComponent() {};

	std::shared_ptr<UniSceneObject> m_CurrentTarget;

	float m_TimeMultiplier = 1.0f;

};

