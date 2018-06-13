#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "UniSceneObject.h"


UniSceneObject::UniSceneObject() {
	m_Position = glm::vec3(0.f, 0.f, 0.f);
	m_Rotation = glm::vec3(0.f, 0.f, 0.f);
	m_Scale = glm::vec3(1.f, 1.f, 1.f);
}

UniSceneObject::~UniSceneObject() {}

void UniSceneObject::SetPosition(const glm::vec3 &pos) {
	SetPosition(pos.x, pos.y, pos.z);

}

void UniSceneObject::SetPosition(float x, float y, float z) {
	m_Position.x = x;
	m_Position.y = y;
	m_Position.z = z;
}

void UniSceneObject::SetPitch(float p) {
	if(p < 0)
		p += 360.f;
	if(p > 360)
		p -= 360.f;

	m_Rotation.x = p;
}

void UniSceneObject::SetYaw(float y) {
	if(y < 0)
		y += 360.f;
	if(y > 360)
		y -= 360.f;

	m_Rotation.y = y;
}

void UniSceneObject::SetRoll(float r) {
	if(r < 0)
		r += 360.f;
	if(r > 360)
		r -= 360.f;

	m_Rotation.z = r;
}

void UniSceneObject::SetScale(const glm::vec3 &scale) {
	m_Scale.x = scale.x;
	m_Scale.y = scale.y;
	m_Scale.z = scale.z;
}

glm::mat4 UniSceneObject::GetModelMat() {
	glm::mat4 mat = glm::mat4(1.0f);

	mat = glm::translate(mat, m_Position);
	mat = glm::scale(mat, m_Scale);

	glm::quat qPitch = glm::angleAxis(glm::radians(m_Rotation.x), glm::vec3(1.f, 0.f, 0.f));
	glm::quat qYaw = glm::angleAxis(glm::radians(m_Rotation.y), glm::vec3(0.f, 1.f, 0.f));
	glm::quat qRoll = glm::angleAxis(glm::radians(m_Rotation.z), glm::vec3(0.f, 0.f, 1.f));

	glm::quat rot = qPitch * qYaw * qRoll;

	mat = mat * glm::toMat4(rot);

	return mat;
}
