#pragma once

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>


struct TransformComponent {

	TransformComponent() :
		m_dPos(glm::dvec3(0.0)),
		m_Rot(glm::vec3(0.f)),
		m_Scale(glm::vec3(1.f)),
		m_Forward(glm::vec3(0, 0, 1.f)),
		m_Up(glm::vec3(0, -1.f, 0)),
		m_Right(glm::vec3(1.f, 0, 0))
	{}
	
	TransformComponent(glm::vec3 pos) : m_dPos((glm::dvec3)pos) {}

	glm::dvec3 m_dPos; // ws
	glm::vec3 m_Forward; // ws
	glm::vec3 m_Up; // ws
	glm::vec3 m_Right; // ws
	glm::vec3 m_Rot; // vec3 euler angles
	glm::vec3 m_Scale; // relative

	void SetPosition(const glm::vec3 &pos) {
		SetPosition(pos.x, pos.y, pos.z);
	}

	void SetPosition(const glm::dvec3 &pos) {
		SetPosition(pos.x, pos.y, pos.z);
	}

	void SetPosition(float x, float y, float z) {
		m_dPos.x = (double)x;
		m_dPos.y = (double)y;
		m_dPos.z = (double)z;
	}

	void SetPosition(double x, double y, double z) {
		m_dPos.x = x;
		m_dPos.y = y;
		m_dPos.z = z;
	}

	void SetPitch(float p) {
		if(p < 0)
			p += 360.f;
		if(p > 360)
			p -= 360.f;

		m_Rot.x = p;
	}

	void SetYaw(float y) {
		if(y < 0)
			y += 360.f;
		if(y > 360)
			y -= 360.f;

		m_Rot.y = y;
	}

	void SetRoll(float r) {
		if(r < 0)
			r += 360.f;
		if(r > 360)
			r -= 360.f;

		m_Rot.z = r;
	}

	void SetScale(const glm::vec3 &scale) {
		m_Scale.x = scale.x;
		m_Scale.y = scale.y;
		m_Scale.z = scale.z;
	}

	void Rotate(glm::vec3 axis, float degrees) {
		auto rotQ = glm::angleAxis(glm::radians(degrees), axis);
		m_Forward = rotQ * m_Forward;
		m_Up = rotQ * m_Up;
		m_Right = rotQ * m_Right;
		m_Rot = glm::degrees(glm::eulerAngles(rotQ));
	}

	void MoveForward(double distance) {
		m_dPos += (glm::dvec3)glm::normalize(m_Forward) * distance;
	}

	void MoveForward(float distance) {
		m_dPos += glm::normalize(m_Forward) * distance;
	}

	void MoveRelative(glm::dvec3 velocity) {
		m_dPos += (glm::dvec3)m_Right * velocity.x;
		m_dPos += (glm::dvec3)m_Up * velocity.y;
		m_dPos += (glm::dvec3)m_Forward * velocity.z;

		//std::cout << "Position now: " << m_dPos.x << ", " << m_dPos.y << ", " << m_dPos.z << std::endl;
	}

	glm::mat4 GetModelMat() {
		glm::dmat4 mat = glm::dmat4(1.0);

		mat = glm::translate(mat, m_dPos) * glm::dmat4(glm::mat3(m_Right, m_Up, m_Forward)) * glm::scale(mat, (glm::dvec3)m_Scale);

		return mat;
	}

};


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

