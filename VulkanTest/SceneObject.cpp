#include "SceneObject.h"


SceneObject::~SceneObject() {
}

void SceneObject::SetPosition(float x, float y, float z) {
	m_Position.x = x;
	m_Position.y = y;
	m_Position.z = z;
}

void SceneObject::SetRotation(float x, float y, float z) {
	m_Rotation.x = x;
	m_Rotation.y = y;
	m_Rotation.z = z;
}

glm::mat4 SceneObject::GetModelMatrix() {
	glm::mat4 mat = glm::mat4(1.0f);

	mat = glm::translate(mat, m_Position);
	mat = glm::scale(mat, m_Scale);

	glm::quat qPitch = glm::angleAxis(glm::radians(m_Rotation.x), glm::vec3(1, 0, 0));
	glm::quat qYaw = glm::angleAxis(glm::radians(m_Rotation.y), glm::vec3(0, 1, 0));
	glm::quat qRoll = glm::angleAxis(glm::radians(m_Rotation.z), glm::vec3(0, 0, 1));

	glm::quat rot = qPitch * qYaw * qRoll;

	mat = mat * glm::toMat4(rot);

	return mat;
}
