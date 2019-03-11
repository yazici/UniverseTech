#pragma once
#include <vulkan/vulkan.h>
#include <map>
#include "ECS.h"
#include "vks/VulkanBuffer.hpp"
#include "vks/VulkanModel.hpp"
#include "vks/VulkanTexture.hpp"

namespace uni
{

  namespace render
  {
    class SceneRenderer;
  }
  namespace components
  {
    class ModelComponent;
  }

  namespace materials
  {
    class Material {
    public:
      Material() = default;
      Material(std::string name);
      virtual void Destroy();
      void RegisterModel(ECS::ComponentHandle<uni::components::ModelComponent> model);
      void UnRegisterModel(ECS::ComponentHandle<uni::components::ModelComponent> model);
      virtual ~Material() = default;

      virtual void SetupDescriptorSetLayout(
        std::shared_ptr<uni::render::SceneRenderer> renderer);
      virtual void PreparePipelines(
        std::shared_ptr<uni::render::SceneRenderer> renderer,
        VkGraphicsPipelineCreateInfo& pipelineCreateInfo);
      virtual void SetupDescriptorPool(std::shared_ptr<uni::render::SceneRenderer> renderer);
      virtual void SetupDescriptorSets(std::shared_ptr<uni::render::SceneRenderer> renderer);

      std::shared_ptr<vks::Buffer> GetBuffer(std::string name);
      void SetBuffer(std::string name, std::shared_ptr<vks::Buffer> buffer);

      std::shared_ptr<vks::Texture2D> GetTexture(std::string name);
      void SetTexture(std::string name, std::shared_ptr<vks::Texture2D> texture);

      std::string GetShader(std::string name) { return m_Shaders.at(name); }
      void SetShader(std::string name, std::string shader) {
        m_Shaders.insert({ name, shader });
      }
      void SetIndexCount(uint32_t count = 0) { m_IndexCount = count; }
      virtual void SetupMaterial(VkGraphicsPipelineCreateInfo& pipelineCreateInfo);
      virtual void LoadTexture(std::string name, std::string texturePath);
      //virtual void AddToCommandBuffer(VkCommandBuffer& cmdBuffer);

      virtual void AddToCommandBuffer(VkCommandBuffer& cmdBuffer,
        ECS::ComponentHandle<uni::components::ModelComponent> model);
      VkDescriptorSet* GetDescriptorSet() { return &m_descriptorSet; }

      std::shared_ptr<uni::render::SceneRenderer> GetSceneRenderer();

      std::string GetName() { return m_Name; }
      void SetName(std::string name) { m_Name = name; }

      void SetBaseColour(glm::vec4 bc) { m_MaterialProperties.baseColour = bc; }
      void SetBaseColour(float r, float g, float b, float a) {
        SetBaseColour({ r, g, b, a });
      }
      void SetEmissiveColour(glm::vec4 bc) {
        // m_MaterialProperties.baseEmissive = bc;
      }
      void SetEmissiveColour(float r, float g, float b, float a) {
        // SetEmissiveColour({r, g, b, a});
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

      void SetUseTexture(bool use) {
        m_MaterialProperties.hasTextureMap = use ? 1 : 0;
      }
      void SetUseNormal(bool use) {
        m_MaterialProperties.hasNormalMap = use ? 1 : 0;
      }
      void SetUseRoughness(bool use) {
        m_MaterialProperties.hasRoughnessMap = use ? 1 : 0;
      }
      void SetUseMetallic(bool use) {
        m_MaterialProperties.hasMetallicMap = use ? 1 : 0;
      }
      void SetUseSpecular(bool use) {
        m_MaterialProperties.hasSpecularMap = use ? 1 : 0;
      }
      void SetUseEmissive(bool use) {
        m_MaterialProperties.hasEmissiveMap = use ? 1 : 0;
      }
      void SetUseAO(bool use) {
        m_MaterialProperties.hasAOMap = use ? 1 : 0;
      }

    protected:
      using MaterialProperties = struct {
        glm::vec4 baseColour = glm::vec4(1.f);  // materials are white by default
        glm::vec4 baseEmissive =
          glm::vec4(0.f, 0.f, 0.f, 1.f);  // materials are not emissive by default
        glm::vec4 baseNormal = glm::vec4(0.f, 0.f, 1.f, 0.f);
        float baseRoughness = 1.f;   // materials are rough by default
        float baseMetallic = 0.f;    // materials are not metallic by default
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

      std::vector<ECS::ComponentHandle<uni::components::ModelComponent>> m_models;

      std::string m_Name;
      uint32_t m_IndexCount = 0;

    public:
      std::map<std::string, std::shared_ptr<vks::Texture2D>> m_Textures;
      std::map<std::string, std::string> m_TexturePaths;

    protected:
      std::map<std::string, std::string> m_Shaders;
      std::map<std::string, std::shared_ptr<vks::Buffer>> m_Buffers;

      VkPipelineLayout m_pipelineLayout = nullptr;
      VkPipeline m_pipeline = nullptr;
      VkDescriptorSetLayout m_descriptorSetLayout = nullptr;
      VkDescriptorSet m_descriptorSet = nullptr;
      std::vector<VkDescriptorSetLayoutBinding> m_setLayoutBindings;
      std::vector<VkWriteDescriptorSet> m_writeDescriptorSets;
      VkDescriptorPool m_descriptorPool = nullptr;

      bool m_setupPerformed = false;
    };
  }
}
