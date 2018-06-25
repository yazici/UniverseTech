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
#include "Transform.h"
#include "ECS.h"

struct CameraComponent {

	float fov;
	float nearClip;
	float farClip;
	float aspect = 1.777778f;

	glm::vec3 m_Position;

	glm::vec3 target;

	struct {
		glm::mat4 projection;
		glm::mat4 view;
	} matrices;

	bool isActive = true;

	enum CameraType {
		CAMERA_FIXED,
		CAMERA_LOOKAT
	};

	CameraType cameraType = CAMERA_FIXED;

	CameraComponent(ECS::ComponentHandle<TransformComponent> transform, float ratio=1.77778f, float verticalFOV=60.f, float nClip=0.1f, float fClip=1000.f) {
		target = { 0, 0, 1 };
		aspect = aspect;
		fov = verticalFOV;
		nearClip = nClip;
		farClip = fClip;
		CalculateProjection();
		CalculateView(transform);
	}


	void CalculateView(ECS::ComponentHandle<TransformComponent> transform) {
		auto mat = transform->GetModelMat();
		m_Position = glm::vec3(mat[3]);

		if(cameraType == CameraType::CAMERA_FIXED) {
			matrices.view = glm::inverse(mat);
		} else {
			auto position = glm::vec3(mat[3]);
			auto up = glm::vec3(mat[1]);
			matrices.view = glm::lookAt(position, target, up);
		}
	}

	void CalculateProjection() {
		matrices.projection = glm::perspective(glm::radians(fov), aspect, nearClip, farClip);
	}

	glm::vec3 GetPosition() {
		return m_Position;
	}



};
