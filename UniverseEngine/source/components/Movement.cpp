#include "Movement.h"

void MovementComponent::ApplyAcceleration(glm::vec3 accel, float deltaTime) {
	accel.x *= m_MaxStrafeAccel;
	accel.y *= m_MaxVerticalAccel;
	if(accel.z > 0)
		accel.z *= m_MaxAccel;
	else
		accel.z *= m_MaxDecel;

	accel.x = std::clamp(accel.x, -m_MaxStrafeAccel, m_MaxStrafeAccel);
	accel.y = std::clamp(accel.y, -m_MaxVerticalAccel, m_MaxVerticalAccel);
	accel.z = std::clamp(accel.z, m_MaxDecel, m_MaxAccel);
	m_dVelocity += accel * deltaTime;
}

void MovementComponent::ApplyTorque(glm::vec3 euler, float deltaTime) {
	euler.x *= m_MaxPitchAccel;
	euler.y *= m_MaxYawAccel;
	euler.z *= m_MaxRollAccel;
	euler.x = std::clamp(euler.x, -m_MaxPitchAccel, m_MaxPitchAccel);
	euler.y = std::clamp(euler.y, -m_MaxYawAccel, m_MaxYawAccel);
	euler.z = std::clamp(euler.z, m_MaxRollAccel, m_MaxRollAccel);
	m_Rotation += euler * deltaTime;
}


void MovementComponent::CrashStop(float deltaTime) {
	m_dVelocity *= 0.0;
	m_Rotation *= 0.0f;
}

void MovementComponent::FullStop(float deltaTime) {
	glm::vec3 accel = -m_dVelocity;
	accel.x = std::clamp(accel.x, -m_MaxStrafeAccel, m_MaxStrafeAccel);
	accel.y = std::clamp(accel.y, -m_MaxVerticalAccel, m_MaxVerticalAccel);
	accel.z = std::clamp(accel.z, m_MaxDecel, m_MaxAccel);
	m_dVelocity += accel * deltaTime;

	glm::vec3 euler = -m_Rotation;
	euler.x = std::clamp(euler.x, -m_MaxPitchAccel, m_MaxPitchAccel);
	euler.y = std::clamp(euler.y, -m_MaxYawAccel, m_MaxYawAccel);
	euler.z = std::clamp(euler.z, m_MaxRollAccel, m_MaxRollAccel);
	m_Rotation += euler * deltaTime;

}

void MovementComponent::ApplyDrag(float deltaTime) {
	glm::vec3 accel = -m_dVelocity;
	accel *= m_Drag;
	m_dVelocity += accel * deltaTime;

	glm::vec3 euler = -m_Rotation;
	euler *= m_RotationalDrag;
	m_Rotation += euler * deltaTime;
}


