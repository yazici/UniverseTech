#pragma once


#include <memory>
#include "../SceneObject.h"
#include "../3dmaths.h"


struct PlayerControlComponent {
	PlayerControlComponent() {};

	std::shared_ptr<SceneObject> m_CurrentTarget;
	glm::vec3 m_TargetPos;
	bool m_HasTarget = false;

	float m_TimeMultiplier = 1.0f;

	bool HasTarget();

	/** @brief sets a target unisceneobject for the player to always track */
	void SetTarget(SceneObject& target);
	/** @brief sets a fixed worldspace target position for the player to always track */
	void SetTarget(const glm::vec3& target);
	/** @brief returns the vec3 worldspace position of the target */
	glm::vec3 GetTargetPos();
	void ClearTarget();

};
