#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "3dmaths.h"

#include <vulkan/vulkan.h>
#include "vks/VulkanBuffer.hpp"
#include "UniModelMesh.h"
#include "vks/VulkanTexture.hpp"
#include "vks/vulkanexamplebase.h"

#define MAX_LIGHT_COUNT 1000

class UniSceneManager;
class UniMaterial;

class UniSceneRenderer {
 protected:
  struct {
    glm::mat4 projection;
    glm::mat4 view;
  } m_uboForward;

  struct UboModelMatDynamic {
    glm::mat4* model = nullptr;
  } m_uboModelMatDynamic;

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

  using PushConstantStruct = struct {
    uint32_t time_seconds = 0;
    uint32_t time_millis = 0;
  };

  PushConstantStruct m_TimeConstants;

  VkDescriptorSetLayout m_descriptorSetLayout;
  VkDescriptorSet m_descriptorSet;
  VkDescriptorSetLayoutCreateInfo m_descriptorLayout;
  std::vector<VkDescriptorSetLayoutBinding> m_setLayoutBindings;
  std::vector<VkWriteDescriptorSet> m_writeDescriptorSets;

  struct {
    VkPipeline forward;  // Forward rendering pipeline
    VkPipeline debug;    // debug display
  } m_pipelines;

  struct {
    VkPipelineLayout forward;
  } m_pipelineLayouts;

  std::map<std::string, std::shared_ptr<UniMaterial>> m_materialInstances;

  VkDescriptorPool m_descriptorPool;

  VkClearColorValue m_defaultClearColor = {{0.025f, 0.025f, 0.025f, 1.0f}};

  // Vertex layout for the models
  uni::VertexLayout m_vertexLayout = uni::VertexLayout({
      uni::VERTEX_COMPONENT_POSITION,
      uni::VERTEX_COMPONENT_UV,
      uni::VERTEX_COMPONENT_COLOR,
      uni::VERTEX_COMPONENT_NORMAL,
      uni::VERTEX_COMPONENT_TANGENT,
      uni::VERTEX_COMPONENT_MATERIAL_ID
  });

  struct {
    VkPipelineVertexInputStateCreateInfo inputState;
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
  } m_vertices;

 public:
  UniSceneRenderer() = default;
  ~UniSceneRenderer() = default;

  void Initialise();
  void ShutDown();
  void Tick(uint32_t millis);
  void Tick(float delta) { Tick(static_cast<uint32_t>(1000 * delta));}

	void SetupVertexDescriptions();
  void PrepareUniformBuffers();

  void Render();
  void ViewChanged();
  void updateUniformBuffersScreen();

  std::shared_ptr<UniSceneManager> SceneManager();
  void UpdateUniformBufferDeferredLights();
  void UpdateDynamicUniformBuffers();

  void UpdateCamera(float width, float height);
  void SetupDescriptorSetLayout();
  void PreparePipelines();
  void SetupDescriptorPool();
  void SetupDescriptorSets();
  void BuildCommandBuffers();
  void RegisterMaterial(std::string materialID, std::shared_ptr<UniMaterial> mat);
  void UnRegisterMaterial(std::string materialID);
  
  template<typename T>
  std::shared_ptr<T> GetMaterialByID(std::string materialID);


  uni::VertexLayout GetVertexLayout() { return m_vertexLayout; }
  VkDescriptorSetLayout GetDescriptorSetLayout() { return m_descriptorSetLayout;}
  VkDescriptorSetLayoutCreateInfo GetDescriptorLayout() {
    return m_descriptorLayout;
  }
  VkDescriptorSet GetDescriptorSet() { return m_descriptorSet; }

  uint32_t GetPushConstantSize() { return sizeof(PushConstantStruct); }
  PushConstantStruct GetPushConstants() { return m_TimeConstants; }

  void AddTimeDelta(uint32_t millis) {
    millis += m_TimeConstants.time_millis;
    m_TimeConstants.time_seconds += millis / 1000;
    m_TimeConstants.time_millis = millis % 1000;
  }

  std::string GetShader(std::string shader);
};

template<typename T>
std::shared_ptr<T> UniSceneRenderer::GetMaterialByID(
    std::string materialID) {
  if (m_materialInstances.find(materialID) == m_materialInstances.end()) {
    auto mat = std::make_shared<T>(materialID);
    RegisterMaterial(materialID, mat);
  }
  return std::dynamic_pointer_cast<T>(m_materialInstances.at(materialID));
}