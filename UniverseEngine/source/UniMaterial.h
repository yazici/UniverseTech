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

  std::shared_ptr<vks::Texture2D> GetTexture(std::string name, short layer);
  void SetTexture(std::string name, short layer, std::shared_ptr<vks::Texture2D> texture);

  std::string GetShader(std::string name) { return m_Shaders.at(name); }
  void SetShader(std::string name, std::string shader) {
    m_Shaders[name] = shader;
  }
  void SetIndexCount(uint32_t count = 0) { m_IndexCount = count; }
  virtual void SetupMaterial(VkGraphicsPipelineCreateInfo& pipelineCreateInfo);
  virtual void LoadTexture(std::string name, short layer, std::string texturePath);
  virtual uint32_t AddToCommandBuffer(VkCommandBuffer& cmdBuffer,
                                      uint32_t index);

  VkDescriptorSet* GetDescriptorSet() { return &m_descriptorSet; }

  std::shared_ptr<UniSceneRenderer> SceneRenderer();

  std::string GetName() { return m_Name; }
  void SetName(std::string name) { m_Name = name; }

  void SetBaseColour(glm::vec3 bc, short layer = 0) { m_MaterialProperties[layer].baseColour = bc; }
  void SetBaseColour(float r, float g, float b, short layer = 0) { SetBaseColour({r, g, b}, layer); }
  void SetEmissiveColour(glm::vec3 bc, short layer = 0) { m_MaterialProperties[layer].emissiveColour = bc; }
  void SetEmissiveColour(float r, float g, float b, short layer = 0) {
    m_MaterialProperties[layer].emissiveColour = {r, g, b};
  }
  void SetRoughness(float roughness, short layer = 0) {
    m_MaterialProperties[layer].roughness = roughness;
  }
  void SetMetallic(float metallic, short layer = 0) {
    m_MaterialProperties[layer].metallic = metallic;
  }
  void SetSpecular(float specular, short layer = 0) {
    m_MaterialProperties[layer].specular = specular;
  }



 protected:

  using MaterialProperties = struct  
  {
    bool hasTextureMap = false;
    bool hasNormalMap = false;
    bool hasRoughnessMap = false;
    bool hasMetallicMap = false;
    bool hasSpecularMap = false;
    bool hasEmissiveMap = false;
    bool hasAOMap = false;
    bool padding = false;

    glm::vec3 baseColour = glm::vec3(1);  // materials are white by default
    glm::vec3 emissiveColour = glm::vec3(0);         // materials are not emissive by default
    float roughness = 1.f;  // materials are smooth by default
    float metallic = 0.f;   // materials are not metallic by default
    float specular = 0.5f;  // materials are plasticy by default

  };

  short layers = 0;

  std::vector<MaterialProperties> m_MaterialProperties;

  std::vector<std::shared_ptr<UniModel>> m_models;

  std::string m_Name;
  uint32_t m_IndexCount;
   
  std::map<std::tuple<std::string, short>, std::shared_ptr<vks::Texture2D>>
      m_Textures;

  std::map<std::string, std::string> m_Shaders;
  std::map<std::string, std::shared_ptr<vks::Buffer>> m_Buffers;

  VkPipelineLayout m_pipelineLayout;
  VkPipeline m_pipeline;
  VkDescriptorSetLayout m_descriptorSetLayout;
  VkDescriptorSet m_descriptorSet;
  std::vector<VkDescriptorSetLayoutBinding> m_setLayoutBindings;
  std::vector<VkWriteDescriptorSet> m_writeDescriptorSets;
  VkDescriptorPool m_descriptorPool;
  


};
