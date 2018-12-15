#include "PhysicsComponent.h"


void PhysicsComponent::AddForce(glm::dvec3 force) {
	AddForceAt(force, glm::dvec3(0));
}

void PhysicsComponent::AddForceAt(glm::dvec3 force, glm::dvec3 pos) {
	m_Velocity += force / m_Mass;
}

void PhysicsComponent::AddTorque(glm::dvec3 torque) {
	auto tQ = glm::angleAxis(glm::length(torque), glm::normalize(torque));

	m_Torque *= tQ;
}


void PhysicsComponent::AddTorque(const glm::vec3 &axis, float angle) {
	auto tQ = glm::angleAxis(angle, axis);

	m_Torque *= tQ;
}
