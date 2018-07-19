#include "Movement.h"
#include <iostream>

void MovementComponent::ApplyAcceleration(glm::vec3 accel, float deltaTime) {
	accel.x *= m_MaxStrafeAccel;
	accel.y *= m_MaxVerticalAccel;
	if(accel.z > 0)
		accel.z *= m_MaxAccel;
	else
		accel.z *= m_MaxDecel;

	accel *= m_BoostFactor;

	m_dVelocity += accel * deltaTime;

	//m_dVelocity = glm::clamp(m_dVelocity, glm::dvec3(-m_MaxStrafe, -m_MaxVertical, -m_MaxReverse), glm::dvec3(m_MaxStrafe, m_MaxVertical, m_MaxSpeed));
}

void MovementComponent::ApplyTorque(glm::vec3 euler, float deltaTime) {
	euler.x *= m_MaxPitchAccel;
	euler.y *= m_MaxYawAccel;
	euler.z *= m_MaxRollAccel;
	/*euler.x = std::clamp(euler.x, -m_MaxPitchAccel, m_MaxPitchAccel);
	euler.y = std::clamp(euler.y, -m_MaxYawAccel, m_MaxYawAccel);
	euler.z = std::clamp(euler.z, -m_MaxRollAccel, m_MaxRollAccel);*/

	//euler *= deltaTime;

	//std::cout << "Setting rotation: " << euler.x << ", " << euler.y << ", " << euler.z << std::endl;

	euler = glm::clamp(euler, glm::vec3(-m_MaxPitch, -m_MaxYaw, -m_MaxRoll), glm::vec3(m_MaxPitch, m_MaxYaw, m_MaxRoll));

	m_Rotation = euler;
}


void MovementComponent::CrashStop(float deltaTime) {
	m_dVelocity *= 0.0;
	m_Rotation *= 0.0f;
}

void MovementComponent::FullStop(float deltaTime) {
	glm::vec3 accel = glm::vec3(-m_MaxStrafeAccel, -m_MaxVerticalAccel, -m_MaxDecel);

	if(m_dVelocity.x < 0) {
		accel.x = m_MaxStrafeAccel;
	}
	if(m_dVelocity.y < 0) {
		accel.y = m_MaxVerticalAccel;
	}
	if(m_dVelocity.z < 0) {
		accel.z = m_MaxAccel;
	}

	m_dVelocity += accel * deltaTime;

	glm::vec3 euler = -m_Rotation;
	
	m_Rotation += euler * deltaTime;

}

void MovementComponent::ApplyDrag(float deltaTime) {
	m_dVelocity *= m_Drag * deltaTime;
	m_Rotation *= m_RotationalDrag * deltaTime;
}

void MovementComponent::SetTarget(glm::vec3 target) {
	m_Target = target;
	m_HasTarget = true;
}

void MovementComponent::SetBoost(float boost) {
	m_BoostFactor = boost;
}

