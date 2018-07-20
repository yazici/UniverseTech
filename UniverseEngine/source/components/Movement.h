#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <algorithm>


struct MovementComponent {
	MovementComponent() : m_dVelocity(glm::dvec3(0.0)), m_Rotation(glm::vec3(0.f)) {}
	MovementComponent(glm::dvec3 vel, glm::vec3 rot = glm::vec3(0)) : m_dVelocity(vel), m_Rotation(rot) {}

	/** @brief units per second */
	glm::dvec3 m_dVelocity; 
	/** @brief degrees per second, pitch yaw roll */
	glm::vec3 m_Rotation; 

	/** @brief units per second */
	float m_MaxSpeed = 20.0f; 
	 /** @brief units per second */
	float m_MaxReverse = 30.0f;
	/** @brief units per second */
	float m_MaxStrafe = 20.0f; 
	/** @brief units per second */
	float m_MaxVertical = 20.0f; 

	/** @brief units per second, per second */
	float m_MaxAccel = 5.0f; 
	/** @brief units per second, per second */
	float m_MaxDecel = 8.0f; 
	/** @brief units per second, per second */
	float m_MaxStrafeAccel = 3.0f; 
	/** @brief units per second, per second */
	float m_MaxVerticalAccel = 3.0f; 
	/** @brief percentage per second, per second */
	float m_Drag = 0.05f; 

	/** @brief degrees per second */
	float m_MaxPitch = 0.0f; 
	/** @brief degrees per second */
	float m_MaxYaw = 90.0f; 
	/** @brief degrees per second */
	float m_MaxRoll = 90.0f; 

	/** @brief degrees per second per second */
	float m_MaxPitchAccel = 720.f;
	/** @brief degrees per second per second */
	float m_MaxYawAccel = 360.f;
	/** @brief degrees per second per second */
	float m_MaxRollAccel = 720.f;
	/** @brief percentage per second per second */
	float m_RotationalDrag = 0.05f;

	/** @brief multiplier for acceleration */
	float m_BoostFactor = 1.0f;


	bool isRelative = true;

	void ApplyAcceleration(glm::vec3 accel, float deltaTime);
	void ApplyTorque(glm::vec3 euler, float deltaTime);
	void CrashStop(float deltaTime = 0.f);
	void FullStop(float deltaTime);
	void ApplyDrag(float deltaTime);
	
	/** @brief set a target specified in world space */
	void SetTarget(glm::vec3 target);
	void SetBoost(float boost);
	glm::vec3 GetTarget() { return m_Target; }
	bool HasTarget() { return m_HasTarget; }
private:
	glm::vec3 m_Target;
	bool m_HasTarget = false;
};

