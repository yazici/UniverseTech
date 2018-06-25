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

struct CameraComponent {

	float fov;
	float nearClip;
	float farClip;
	float aspect = 1.777778f;

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

	CameraComponent(ECS::ComponentHandle<TransformComponent> transform) {
		target = { 0, 0, 1 };
		CalculateProjection();
		CalculateView(transform);
	}


	void CalculateView(ECS::ComponentHandle<TransformComponent> transform) {
		auto mat = transform->GetModelMat();
		if(cameraType == CameraType::CAMERA_FIXED) {
			matrices.view = mat;
		} else {
			auto position = glm::vec3(mat[3]);
			auto up = glm::vec3(mat[1]);
			matrices.view = glm::lookAt(position, target, up);
		}
	}

	void CalculateProjection() {
		matrices.projection = glm::perspective(fov, aspect, nearClip, farClip);
	}



};
