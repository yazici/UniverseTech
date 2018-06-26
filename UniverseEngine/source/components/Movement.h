#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>


struct MovementComponent {
	MovementComponent() : m_dVelocity(glm::dvec3(0.0)), m_Rotation(glm::vec3(0.f)), m_RotationSpeed(float(0.f)) {}
	MovementComponent(glm::dvec3 vel, glm::vec3 rot = glm::vec3(0), float rotSpeed = 0.f) : m_dVelocity(vel), m_Rotation(rot), m_RotationSpeed(rotSpeed) {}

	glm::dvec3 m_dVelocity;
	glm::vec3 m_Rotation;
	float m_RotationSpeed;

	bool isRelative = true;

	glm::dvec3 m_dNewPosition;
	glm::vec4 m_NewRotation;

};

