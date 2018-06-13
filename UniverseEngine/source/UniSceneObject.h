#pragma once
#include "glm/glm.hpp"

class UniSceneObject {
public:
	UniSceneObject();
	virtual ~UniSceneObject();

	glm::vec3 m_Position;
	glm::vec3 m_Rotation;
	glm::vec3 m_Scale;

	glm::mat4 m_ModelMat;

	void SetPosition(const glm::vec3 &pos);
	void SetPosition(float x, float y, float z);
	void SetPitch(float p);
	void SetYaw(float y);
	void SetRoll(float r);
	void SetScale(const glm::vec3 &scale);

	glm::mat4 GetModelMat();

};

