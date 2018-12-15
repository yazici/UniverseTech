#pragma once


#include <iostream>
#include "../ECS.h"
#include "../3dmaths.h"


class UniSceneObject;

struct TransformComponent {

	TransformComponent() :
		m_dPos(glm::dvec3(0.0)),
		m_Scale(glm::vec3(1.f)),
		m_Forward(glm::vec3(0, 0, 1.f)),
		m_Up(glm::vec3(0, 1.f, 0)),
		m_Right(glm::vec3(1.f, 0, 0)) {

		m_Rotation = glm::quat(glm::mat3(m_Right, m_Up, m_Forward));
	}

	TransformComponent(glm::vec3 pos):
		m_dPos(glm::dvec3(pos)),
		m_Scale(glm::vec3(1.f)),
		m_Forward(glm::vec3(0, 0, 1.f)),
		m_Up(glm::vec3(0, 1.f, 0)),
		m_Right(glm::vec3(1.f, 0, 0)) {

		m_Rotation = glm::quat(glm::mat3(m_Right, m_Up, m_Forward)); 
	}

	glm::dvec3 m_dPos; // relative to parent, no parent == world space
	glm::vec3 m_Scale;
	glm::quat m_Rotation = glm::quat();

	glm::vec3 m_Forward;
	glm::vec3 m_Up;
	glm::vec3 m_Right;

	std::shared_ptr<UniSceneObject> m_Parent;

	void SetParent(std::shared_ptr<UniSceneObject> parent);

	void SetPosition(const glm::vec3 &pos);

	glm::vec3 TransformLocalToWS(glm::vec3 localPos);

	glm::vec3 GetPosition();

	glm::vec3 TransformWSToLocal(glm::vec3 wsPos);

	glm::vec3 TransformWSToObject(glm::vec3 wsPos);

	void SetPosition(const glm::dvec3 &pos);

	void SetPosition(float x, float y, float z);

	void SetPosition(double x, double y, double z);

	void SetRotation(const glm::dvec3 &angleAxis);

	void SetScale(const glm::vec3 &scale);

	void Rotate(glm::vec3 axis, float degrees);

	void Rotate(glm::vec3 euler);

	void Rotate(glm::quat q);

	void RotateToTarget(glm::vec3 target);

	void MoveForward(double distance);

	void MoveForward(float distance);

	void MoveWorld(glm::dvec3 velocity);
	void MoveRelative(glm::dvec3 velocity);

	glm::mat4 GetModelMat();

	glm::mat4 GetObjectMat();

};
