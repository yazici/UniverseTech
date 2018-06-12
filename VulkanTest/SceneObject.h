#pragma once

#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include "VulkanCommon.h"

class SceneObject {
public:
	virtual ~SceneObject();
	glm::vec3 GetPosition() { return m_Position; }
	glm::vec3 GetRotation() { return m_Rotation; }
	glm::vec3 GetScale() { return m_Scale; }

	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);

	glm::mat4 GetModelMatrix();

protected:
	glm::vec3 m_Position;
	glm::vec3 m_Rotation;
	glm::vec3 m_Scale;
};

#endif