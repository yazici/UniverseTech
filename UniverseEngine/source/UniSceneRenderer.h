#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "3dmaths.h"

#include <vulkan/vulkan.h>
#include "vks/VulkanBuffer.hpp"
#include "vks/VulkanModel.hpp"
#include "vks/VulkanTexture.hpp"
#include "vks/vulkanexamplebase.h"

#define MAX_LIGHT_COUNT 1000

class UniSceneManager;

class UniSceneRenderer {
 public:
  struct {
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 model;
  } uboForward;

  struct UboModelMatDynamic {
    glm::mat4* model = nullptr;
  } uboModelMatDynamic;

  struct Light {
    glm::vec4 position;
    glm::vec3 color;
    float radius;
  };

  struct {
    Light lights[MAX_LIGHT_COUNT];
    glm::vec4 viewPos;
    uint32_t numLights;
  } uboLights;

  struct {
    vks::Buffer vsForward;
    vks::Buffer fsLights;
    vks::Buffer modelViews;
  } m_uniformBuffers;

public:
  UniSceneRenderer() = default;
  ~UniSceneRenderer() = default;

	void Initialise();
  void ShutDown();

  void prepareUniformBuffers();

  void Render();
  void ViewChanged();
  void updateUniformBuffersScreen();

  std::shared_ptr<UniSceneManager> SceneManager();
  void updateUniformBufferDeferredLights();
  void updateDynamicUniformBuffers();

  void UpdateCamera(float width, float height);
};
