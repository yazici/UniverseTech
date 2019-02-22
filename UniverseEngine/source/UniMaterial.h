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

  void SetBaseColour(glm::vec4 bc) { m_MaterialProperties.baseColour = bc; }
  void SetBaseColour(float r, float g, float b, float a) { SetBaseColour({r, g, b, a}); }
  void SetEmissiveColour(glm::vec4 bc) { 
    //m_MaterialProperties.baseEmissive = bc; 
  }
  void SetEmissiveColour(float r, float g, float b, float a) {
    //SetEmissiveColour({r, g, b, a});
  }
  void SetRoughness(float roughness) {
    m_MaterialProperties.baseRoughness = roughness;
  }
  void SetMetallic(float metallic) {
    m_MaterialProperties.baseMetallic = metallic;
  }
  void SetSpecular(float specular) {
    m_MaterialProperties.baseSpecular = specular;
  }



 protected:

  using MaterialProperties = struct  
  {
    glm::vec4 baseColour = glm::vec4(1.f);  // materials are white by default
    glm::vec4 baseEmissive =
        glm::vec4(0.f, 0.f, 0.f, 1.f);  // materials are not emissive by default
    glm::vec4 baseNormal = glm::vec4(0.f, 0.f, 1.f, 0.f);
    float baseRoughness = 1.f;  // materials are rough by default
    float baseMetallic = 0.f;   // materials are not metallic by default
    float baseSpecular = 0.04f;  // materials are plasticy by default
    unsigned int hasTextureMap = 0;
    unsigned int hasNormalMap = 0;
    unsigned int hasRoughnessMap = 0;
    unsigned int hasMetallicMap = 0;
    unsigned int hasSpecularMap = 0;
    unsigned int hasEmissiveMap = 0;
    unsigned int hasAOMap = 0;
    unsigned int isVisible = 1;
  };


  MaterialProperties m_MaterialProperties;
  vks::Buffer m_MaterialPropertyBuffer;

  std::vector<std::shared_ptr<UniModel>> m_models;

  std::string m_Name;
  uint32_t m_IndexCount;
   
  std::map<std::string, std::shared_ptr<vks::Texture2D>>
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
