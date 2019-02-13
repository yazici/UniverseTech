#pragma once
#include <vulkan/vulkan.h>
#include <map>
#include "factory.h"
#include "vks/VulkanBuffer.hpp"
#include "vks/VulkanModel.hpp"
#include "vks/VulkanTexture.hpp"

class UniSceneRenderer;

class UniMaterial {
 public:

  UniMaterial() = default;
  virtual void Destroy();
  virtual ~UniMaterial() { Destroy(); }

  VkPipeline m_Pipeline;

  std::map<std::string, std::shared_ptr<vks::Buffer>> m_Buffers;
  std::shared_ptr<vks::Buffer> GetBuffer(std::string buffer);
  void SetBuffer(std::string name, std::shared_ptr<vks::Buffer> buffer);

  std::map<std::string, std::string> m_Shaders;
  std::string GetShader(std::string name) { return m_Shaders.at(name); }
  void SetShader(std::string name, std::string shader) {
    m_Shaders[name] = shader;
  }
  void SetIndexCount(uint32_t count = 0) { m_IndexCount = count; }
  virtual void SetupMaterial(
      VkGraphicsPipelineCreateInfo& pipelineCreateInfo);
  virtual uint32_t AddToCommandBuffer(VkCommandBuffer& cmdBuffer, uint32_t index, VkPipelineLayout layout) = 0;

  std::shared_ptr<UniSceneRenderer> SceneRenderer();

protected:
  std::string m_Name;
  uint32_t m_IndexCount;
};
