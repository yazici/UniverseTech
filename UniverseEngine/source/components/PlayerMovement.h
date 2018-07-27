#pragma once

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <memory>
#include "../UniSceneObject.h"


struct PlayerControlComponent {
	PlayerControlComponent() {};

	std::shared_ptr<UniSceneObject> m_CurrentTarget;
	glm::vec3 m_TargetPos;
	bool m_HasTarget = false;

	float m_TimeMultiplier = 1.0f;

	bool HasTarget();

	/** @brief sets a target unisceneobject for the player to always track */
	void SetTarget(UniSceneObject& target);
	/** @brief sets a fixed worldspace target position for the player to always track */
	void SetTarget(const glm::vec3& target);
	/** @brief returns the vec3 worldspace position of the target */
	glm::vec3 GetTargetPos();
	void ClearTarget();

};
