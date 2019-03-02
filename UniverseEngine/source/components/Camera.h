#pragma once
#ifndef _CAMERA_HEADER_
#define _CAMERA_HEADER_

#include <iostream>
#include "../3dmaths.h"
#include "../ECS.h"
#include "Transform.h"

struct CameraComponent {
  float fov;
  float nearClip;
  float farClip;
  float aspect = 1.777778f;

  glm::vec3 m_Position = glm::vec3(0);

  glm::vec3 target;

  struct {
    glm::mat4 projection = glm::identity<glm::mat4>();
    glm::mat4 view = glm::identity<glm::mat4>();
  } matrices;

  bool isActive = true;

  enum CameraType { CAMERA_FIXED, CAMERA_LOOKAT };

  CameraType cameraType = CAMERA_FIXED;

  ECS::ComponentHandle<TransformComponent> m_Transform;

  CameraComponent(ECS::ComponentHandle<TransformComponent> transform,
                  float ratio = 1.77778f,
                  float verticalFOV = 50.f,
                  float nClip = 0.1f,
                  float fClip = 1000.f) {
    target = glm::vec3(0.0, 0.0, 1.0);
    aspect = aspect;
    fov = verticalFOV;
    nearClip = nClip;
    farClip = fClip;
    m_Transform = transform;
    CalculateProjection();
    CalculateView(transform);
  }

  ECS::ComponentHandle<TransformComponent> GetTransform() { return m_Transform; }

  void UpdateTarget(glm::vec3 t) { target = t; }

  void CalculateView(ECS::ComponentHandle<TransformComponent> transform) {
    auto mat = transform->GetModelMat();

    m_Position = {mat[3].x, mat[3].y, mat[3].z};
    auto forward = m_Position + transform->TransformLocalDirectionToWorldSpace(glm::vec3(0.0, 0.0, 1.0));
    //forward = glm::normalize(forward);
    auto up = glm::mat3(mat) * glm::vec3(0.0, 1.0, 0.0);
    //up = glm::normalize(up);

    if (cameraType == CameraType::CAMERA_FIXED) {
      target = forward;
    }
    matrices.view = glm::lookAt(m_Position, target, up);

    //std::cout << "Camera view matrix: " << glm::to_string<glm::mat4>(matrices.view) << std::endl;

  }

  void CalculateProjection() {
    // matrices.projection = glm::perspective(glm::radians(fov), aspect,
    // nearClip, farClip);

    auto fov_radians = glm::radians(fov);

    float f = 1.0f / tan(fov_radians / 2.0f);
    matrices.projection =
        glm::mat4(
          f / aspect, 0.0f, 0.0f, 0.0f, 
          0.0f, f, 0.0f, 0.0f, 
          0.0f, 0.0f, 0.0f, -1.0f, 
          0.0f, 0.0f, nearClip, 0.0f);
  }

  glm::vec3 GetPosition() { return m_Position; }
};

#endif