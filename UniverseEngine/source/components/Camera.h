#pragma once
#ifndef _CAMERA_HEADER_
#define _CAMERA_HEADER_


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
#include "../ECS.h"


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

	void UpdateTarget(glm::vec3 t) {
		target = t;
	}


	void CalculateView(ECS::ComponentHandle<TransformComponent> transform) {
		auto mat = transform->GetModelMat();
		
		m_Position = mat[3];
		auto forward = transform->TransformLocalToWS(glm::vec3(0, 0, 1));
		auto up = glm::mat3(mat) * glm::vec3(0, 1, 0);

		if(cameraType == CameraType::CAMERA_FIXED) {
			target = forward;
		}
		matrices.view = glm::lookAt(m_Position, target, up);
	}

	void CalculateProjection() {
		//matrices.projection = glm::perspective(glm::radians(fov), aspect, nearClip, farClip);
		
		auto fov_radians = glm::radians(fov);

		float f = 1.0f / tan(fov_radians / 2.0f);
		matrices.projection = glm::mat4(
			f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, -1.0f,
			0.0f, 0.0f, nearClip, 0.0f);

		
	}

	glm::vec3 GetPosition() {
		return m_Position;
	}



};

#endif