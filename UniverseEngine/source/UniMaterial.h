#pragma once
#include <vulkan/vulkan.h>
#include <map>
#include "factory.h"
#include "vks/VulkanBuffer.hpp"
#include "vks/VulkanModel.hpp"
#include "vks/VulkanTexture.hpp"

class UniSceneRenderer;
class UniModel;

class UniMaterial {
 public:
  UniMaterial() = default;
  virtual void Destroy();
  void RegisterModel(std::shared_ptr<UniModel> model);
  void UnRegisterModel(UniModel* model);
  virtual ~UniMaterial() { Destroy(); }

  virtual void SetupDescriptorSetLayout(std::shared_ptr<UniSceneRenderer> renderer);
  virtual void PreparePipelines(
      std::shared_ptr<UniSceneRenderer> renderer,
      VkGraphicsPipelineCreateInfo& pipelineCreateInfo);
  virtual void SetupDescriptorPool(std::shared_ptr<UniSceneRenderer> renderer);
  virtual void SetupDescriptorSets(std::shared_ptr<UniSceneRenderer> renderer);

  std::shared_ptr<vks::Buffer> GetBuffer(std::string name);
  void SetBuffer(std::string name, std::shared_ptr<vks::Buffer> buffer);

  std::shared_ptr<vks::Texture2D> GetTexture(std::string name);
  void SetTexture(std::string name, std::shared_ptr<vks::Texture2D> texture);

  std::string GetShader(std::string name) { return m_Shaders.at(name); }
  void SetShader(std::string name, std::string shader) {
    m_Shaders[name] = shader;
  }
  void SetIndexCount(uint32_t count = 0) { m_IndexCount = count; }
  virtual void SetupMaterial(VkGraphicsPipelineCreateInfo& pipelineCreateInfo);
  virtual void LoadTexture(std::string name, std::string texturePath);
  virtual uint32_t AddToCommandBuffer(VkCommandBuffer& cmdBuffer,
                                      uint32_t index);

  VkDescriptorSet* GetDescriptorSet() { return &m_descriptorSet; }

  std::shared_ptr<UniSceneRenderer> SceneRenderer();

  std::string GetName() { return m_Name; }
  void SetName(std::string name) { m_Name = name; }

  void SetBaseColour(glm::vec3 bc) { m_baseColour = bc; }
  void SetBaseColour(float r, float g, float b) { m_baseColour = {r, g, b}; }
  void SetEmissiveColour(glm::vec3 bc) { m_emissiveColour = bc; }
  void SetEmissiveColour(float r, float g, float b) { m_emissiveColour = {r, g, b}; }
  void SetRoughness(float roughness) { m_roughness = roughness; }
  void SetMetallic(float metallic) { m_metallic = metallic; }
  void SetSpecular(float specular) { m_specular = specular; }



 protected:

  struct  
  {
    bool hasTextureMap = false;
    bool hasNormalMap = false;
    bool hasRoughnessMap = false;
    bool hasMetallicMap = false;
    bool hasSpecularMap = false;
    bool hasEmissiveMap = false;
    bool hasAOMap = false;
    bool padding = false;
  } m_MaterialProperties;

  std::vector<std::shared_ptr<UniModel>> m_models;

  bool m_hasTextureMap = false;
  bool m_hasNormalMap = false;
  bool m_hasRoughnessMap = false;
  bool m_hasSpecularMap = false;
  bool m_hasMetallicMap = false;
  bool m_hasEmissiveMap = false;
  bool m_hasAOMap = false;

  std::string m_Name;
  uint32_t m_IndexCount;
  std::map<std::string, std::shared_ptr<vks::Texture2D>> m_Textures;

  std::map<std::string, std::string> m_Shaders;
  std::map<std::string, std::shared_ptr<vks::Buffer>> m_Buffers;

  VkPipelineLayout m_pipelineLayout;
  VkPipeline m_pipeline;
  VkDescriptorSetLayout m_descriptorSetLayout;
  VkDescriptorSet m_descriptorSet;
  std::vector<VkDescriptorSetLayoutBinding> m_setLayoutBindings;
  std::vector<VkWriteDescriptorSet> m_writeDescriptorSets;
  VkDescriptorPool m_descriptorPool;
  

  glm::vec3 m_baseColour = glm::vec3(1); // materials are white by default
  glm::vec3 m_emissiveColour = glm::vec3(0); // materials are not emissive by default
  float m_roughness = 1.f; // materials are smooth by default
  float m_metallic = 0.f; // materials are not metallic by default
  float m_specular = 0.5f; // materials are plasticy by default

};
