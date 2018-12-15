#include "PhysicsComponent.h"


void PhysicsComponent::AddForce(glm::dvec3 force) {
	AddForceAt(force, glm::dvec3(0));
}

void PhysicsComponent::AddForceAt(glm::dvec3 force, glm::dvec3 pos) {
	m_Velocity += force / m_Mass;
}

void PhysicsComponent::AddAngularVelocity(const glm::vec3 &angular) {

	m_AngularVelocity += angular;
}
