#include "Transform.h"
#include "../UniSceneObject.h"

void TransformComponent::SetParent(std::shared_ptr<UniSceneObject> parent) {
	m_Parent = parent;
};

void TransformComponent::SetPosition(const glm::vec3 &pos) {
	SetPosition(pos.x, pos.y, pos.z);
}

glm::vec3 TransformComponent::TransformLocalToWS(glm::vec3 localPos) {
	return glm::vec3(GetModelMat() * glm::vec4(localPos, 1));
}

glm::vec3 TransformComponent::TransformWSToLocal(glm::vec3 wsPos) {
	return glm::vec3(glm::inverse(GetModelMat()) * glm::vec4(wsPos, 1.0));
}

void TransformComponent::SetPosition(const glm::dvec3 &pos) {
	SetPosition(pos.x, pos.y, pos.z);
}

void TransformComponent::SetPosition(float x, float y, float z) {
	m_dPos.x = (double)x;
	m_dPos.y = (double)y;
	m_dPos.z = (double)z;
}

void TransformComponent::SetPosition(double x, double y, double z) {
	m_dPos.x = x;
	m_dPos.y = y;
	m_dPos.z = z;
}

void TransformComponent::SetPitch(float p) {
	if(p < 0)
		p += 360.f;
	if(p > 360)
		p -= 360.f;

	m_Rot.x = p;
}

void TransformComponent::SetYaw(float y) {
	if(y < 0)
		y += 360.f;
	if(y > 360)
		y -= 360.f;

	m_Rot.y = y;
}

void TransformComponent::SetRoll(float r) {
	if(r < 0)
		r += 360.f;
	if(r > 360)
		r -= 360.f;

	m_Rot.z = r;
}

void TransformComponent::SetScale(const glm::vec3 &scale) {
	m_Scale.x = scale.x;
	m_Scale.y = scale.y;
	m_Scale.z = scale.z;
}

void TransformComponent::Rotate(glm::vec3 axis, float degrees) {
	auto rotQ = glm::angleAxis(glm::radians(degrees), axis);
	m_Forward = rotQ * m_Forward;
	m_Up = rotQ * m_Up;
	m_Right = rotQ * m_Right;
	m_Rot = glm::degrees(glm::eulerAngles(rotQ));
}

void TransformComponent::MoveForward(double distance) {
	m_dPos += (glm::dvec3)glm::normalize(m_Forward) * distance;
}

void TransformComponent::MoveForward(float distance) {
	m_dPos += glm::normalize(m_Forward) * distance;
}

void TransformComponent::MoveRelative(glm::dvec3 velocity) {
	m_dPos += (glm::dvec3)m_Right * velocity.x;
	m_dPos += (glm::dvec3)m_Up * velocity.y;
	m_dPos += (glm::dvec3)m_Forward * velocity.z;

	//std::cout << "Position now: " << m_dPos.x << ", " << m_dPos.y << ", " << m_dPos.z << std::endl;
}

glm::mat4 TransformComponent::GetModelMat() {

	glm::dmat4 mat = glm::dmat4(1.0);
	mat = glm::translate(mat, m_dPos) * glm::dmat4(glm::mat3(m_Right, m_Up, m_Forward)) * glm::scale(mat, (glm::dvec3)m_Scale);

	// TODO: this is not picking up rotation from parent!!!
	if(m_Parent) {
		mat = glm::dmat4(m_Parent->GetTransform()->GetModelMat()) * mat;
	}

	return mat;
}