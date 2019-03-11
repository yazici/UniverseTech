#include "Transform.h"
#include "../SceneObject.h"

void TransformComponent::SetParent(std::shared_ptr<SceneObject> parent) {
	m_Parent = parent;
};

void TransformComponent::SetPosition(const glm::vec3 &pos) {
	SetPosition(pos.x, pos.y, pos.z);
}

void TransformComponent::SetRotation(const glm::dvec3 &angleAxis) {
	auto degrees = glm::length(angleAxis);
	auto axis = glm::normalize(angleAxis);
	if(degrees != 0.0 && glm::length(axis) != 0.0)
		m_Rotation = glm::angleAxis(glm::radians(degrees), axis);
}

glm::vec3 TransformComponent::TransformLocalToWS(glm::vec3 localPos) {
	return glm::vec3(GetModelMat() * glm::vec4(localPos, 1));
}

glm::vec3 TransformComponent::GetPosition() {
	return TransformLocalToWS(glm::vec3(0));
}

glm::vec3 TransformComponent::TransformWSToLocal(glm::vec3 wsPos) {
	return glm::vec3(glm::inverse(GetModelMat()) * glm::vec4(wsPos, 1.0));
}

glm::vec3 TransformComponent::TransformLocalDirectionToWorldSpace(glm::vec3 wsPos) {
	return glm::rotate(GetRotation(), wsPos);
}


glm::vec3 TransformComponent::TransformWSToObject(glm::vec3 wsPos) {
	return glm::vec3(glm::inverse(GetObjectMat()) * glm::vec4(wsPos, 1.0));
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

void TransformComponent::SetScale(const glm::vec3 &scale) {
	m_Scale.x = scale.x;
	m_Scale.y = scale.y;
	m_Scale.z = scale.z;
}

void TransformComponent::Rotate(glm::vec3 axis, float degrees) {
	auto rotQ = glm::angleAxis(glm::radians(degrees), axis);
	Rotate(rotQ);
}

void TransformComponent::Rotate(glm::dvec3 angular) {
	auto rotQ = glm::quat(angular);
	Rotate(rotQ);
}


void TransformComponent::Rotate(glm::vec3 euler) {
	auto rotQ = glm::quat(glm::radians(euler));
	Rotate(rotQ);
}

void TransformComponent::Rotate(glm::quat q) {
	// TODO: This is not working!!!
	// FIXME: I am BROKEN

	m_Rotation *= q;

	glm::mat3 m = glm::mat3(m_Rotation);
	m_Right = m[0];
	m_Up = m[1];
	m_Forward = m[2];
}

void TransformComponent::RotateToTarget(glm::vec3 target) {
	glm::vec3 lsTarget = TransformWSToObject(target);
	glm::vec3 newForward = glm::normalize(lsTarget);
	//newForward.y = m_Forward.y;
	std::cout << "rotating to face: " << newForward.x << ", " << newForward.y << ", " << newForward.z << std::endl;
	auto rotQ = glm::rotation(glm::normalize(m_Forward), glm::normalize(newForward));
	m_Forward = rotQ * m_Forward;
	m_Up = rotQ * m_Up;
	m_Right = rotQ * m_Right;

	m_Rotation = glm::quat(glm::mat3(m_Right, m_Up, m_Forward));
}

void TransformComponent::MoveForward(double distance) {
	m_dPos += (glm::dvec3)glm::normalize(m_Forward) * distance;
}

void TransformComponent::MoveForward(float distance) {
	m_dPos += glm::normalize(m_Forward) * distance;
}

void TransformComponent::MoveWorld(glm::dvec3 velocity) {
	m_dPos += velocity;

	//std::cout << "Position now: " << m_dPos.x << ", " << m_dPos.y << ", " << m_dPos.z << std::endl;
}

void TransformComponent::MoveRelative(glm::dvec3 velocity) {
	m_dPos += (glm::dvec3)m_Right * velocity.x;
	m_dPos += (glm::dvec3)m_Up * velocity.y;
	m_dPos += (glm::dvec3)m_Forward * velocity.z;

	//std::cout << "Position now: " << m_dPos.x << ", " << m_dPos.y << ", " << m_dPos.z << std::endl;
}

glm::quat TransformComponent::GetRotation() {
	auto rot = m_Rotation;
	if(m_Parent) {
		rot *= m_Parent->GetTransform()->GetRotation();
	}

	return rot;
}

glm::mat4 TransformComponent::GetModelMat() {

	glm::dmat4 mat = glm::dmat4(1.0);
	mat = glm::translate(mat, m_dPos) * glm::dmat4(glm::mat3(m_Right, m_Up, m_Forward)) * glm::scale(mat, (glm::dvec3)m_Scale);

	if(m_Parent) {
		mat = glm::dmat4(m_Parent->GetTransform()->GetModelMat()) * mat;
	}

	return mat;
}

glm::mat4 TransformComponent::GetObjectMat() {

	glm::dmat4 mat = glm::dmat4(1.0);

	if(m_Parent) {
		mat = glm::dmat4(m_Parent->GetTransform()->GetModelMat()) * mat;
	}

	return mat;

}
