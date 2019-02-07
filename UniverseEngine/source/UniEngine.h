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

#define VERTEX_BUFFER_BIND_ID 0
#define INSTANCE_BUFFER_BIND_ID 1
#define ENABLE_VALIDATION false
// todo: check if hardware supports sample number (or select max. supported)
#define SAMPLE_COUNT VK_SAMPLE_COUNT_8_BIT


// forward declarations
class UniMaterial;
class UniModel;
class UniSceneManager;
class UniSceneRenderer;
class UniInput;

class UniEngine final : public VulkanExampleBase {
 private:
  UniEngine();
  ~UniEngine();
  UniEngine(const UniEngine&) = delete;
  UniEngine& operator=(const UniEngine&) = delete;
  UniEngine(UniEngine&&) = delete;
  UniEngine& operator=(UniEngine&&) = delete;

 public:
  static UniEngine& GetInstance();

  // Vertex layout for the models
  vks::VertexLayout vertexLayout = vks::VertexLayout({
      vks::VERTEX_COMPONENT_POSITION,
      vks::VERTEX_COMPONENT_UV,
      vks::VERTEX_COMPONENT_COLOR,
      vks::VERTEX_COMPONENT_NORMAL,
      vks::VERTEX_COMPONENT_TANGENT,
  });

  struct {
    VkPipelineVertexInputStateCreateInfo inputState;
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
  } vertices;


  struct {
    VkPipeline forward;  // Forward rendering pipeline
    VkPipeline debug;    // debug display
  } pipelines;

  struct {
    VkPipelineLayout forward;
  } pipelineLayouts;

  VkDescriptorSet m_descriptorSet;
  VkDescriptorSetLayout m_descriptorSetLayout;

  std::vector<VkDescriptorSetLayoutBinding> m_setLayoutBindings;
  std::vector<VkWriteDescriptorSet> m_writeDescriptorSets;

  // Framebuffer for offscreen rendering
  struct FrameBufferAttachment {
    VkImage image;
    VkDeviceMemory mem;
    VkImageView view;
    VkFormat format;
  };

  void getEnabledFeatures() override;
  void buildCommandBuffers() override;
  void setupVertexDescriptions();
  void setupDescriptorPool();
  void setupDescriptorSetLayout();
  void setupDescriptorSets();
  void preparePipelines();
  size_t getDynamicAlignment();
  void draw();
  void prepare() override;
  void render() override;
  void viewChanged() override;
  void OnUpdateUIOverlay(vks::UIOverlay* overlay) override;
  void ToggleWireframe();

  void RegisterMaterial(std::shared_ptr<UniMaterial> mat);
  void UnRegisterMaterial(std::shared_ptr<UniMaterial> mat);

  VkDevice GetDevice() { return device; }
  VkQueue GetQueue() { return queue; }
  VkPipelineCache GetPipelineCache() { return pipelineCache; }
  void Shutdown();
  bool m_debugDisplay = false;
  bool m_useMSAA = true;
  bool m_useSampleShading = true;
  bool m_useWireframe = false;

  // One sampler for the frame buffer color attachments
  VkSampler m_colorSampler;

  VkCommandBuffer m_forwardCommandBuffer = VK_NULL_HANDLE;

 private:
  std::shared_ptr<UniSceneManager> m_SceneManager;
  std::shared_ptr<UniInput> m_InputManager;
  std::shared_ptr<UniSceneRenderer> m_SceneRenderer;


  std::vector<std::shared_ptr<UniMaterial>> m_MaterialInstances;
  bool m_CamPaused = false;
  float m_PlanetZOffset = 0;

 public:
  std::shared_ptr<UniSceneManager> SceneManager() { return m_SceneManager; }
  std::shared_ptr<UniSceneRenderer> SceneRenderer() { return m_SceneRenderer; }
  void windowResized() override;
  std::shared_ptr<UniInput> GetInputManager() { return m_InputManager; }
  void SetupInput();

  void handleWMMessages(MSG& msg) override;

  void updateOverlay() override;
  void OnUpdateUserUIOverlay(vks::UIOverlay* overlay);

};
