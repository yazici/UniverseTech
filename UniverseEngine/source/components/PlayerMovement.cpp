#include "PlayerMovement.h"

bool PlayerControlComponent::HasTarget() {
	return m_HasTarget;
}

void PlayerControlComponent::SetTarget(UniSceneObject& target) {
	m_CurrentTarget = std::make_shared<UniSceneObject>(target);
	m_HasTarget = true;
}

void PlayerControlComponent::SetTarget(const glm::vec3& target) {
	m_TargetPos = target;
	m_HasTarget = true;
}

glm::vec3 PlayerControlComponent::GetTargetPos() {
	if(m_CurrentTarget)
		return m_CurrentTarget->GetTransform()->m_dPos;
	else
		return m_TargetPos;
}

void PlayerControlComponent::ClearTarget() {
	m_CurrentTarget.reset();
	m_HasTarget = false;
}
