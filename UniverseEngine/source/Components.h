#pragma once

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


struct Transform {

	Transform() : m_dPos(glm::dvec3(0.0)), m_Rot(glm::vec3(0.f)), m_Scale(glm::vec3(1.f)) {}
	Transform(glm::vec3 pos) : m_dPos((glm::dvec3)pos) {}

	glm::dvec3 m_dPos;

	glm::vec3 m_Rot;
	glm::vec3 m_Scale;

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

	glm::mat4 GetModelMat() {
		glm::dmat4 mat = glm::dmat4(1.0);

		mat = glm::translate(mat, m_dPos);
		mat = glm::scale(mat, (glm::dvec3)m_Scale);

		glm::quat qPitch = glm::angleAxis(glm::radians(m_Rot.x), glm::vec3(1.f, 0.f, 0.f));
		glm::quat qYaw = glm::angleAxis(glm::radians(m_Rot.y), glm::vec3(0.f, 1.f, 0.f));
		glm::quat qRoll = glm::angleAxis(glm::radians(m_Rot.z), glm::vec3(0.f, 0.f, 1.f));

		glm::quat rot = qPitch * qYaw * qRoll;

		mat = (glm::mat4)mat * glm::toMat4(rot);

		return mat;
	}

};

