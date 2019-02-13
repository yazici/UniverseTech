#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include "3dmaths.h"
#include "UniSceneObject.h"
#include "vks/VulkanModel.hpp"
#include "vks/VulkanTexture.hpp"
#include "vulkan/vulkan_core.h"
#include "materials/ModelMaterial.h" 

using json = nlohmann::json;

class UniModel : public UniSceneObject {
 public:
  UniModel() { m_Name = "unnamed model"; }
  UniModel(std::string n);
  void Destroy();
  ~UniModel();

  UniModel(std::string n,
           const std::string& modelpath,
           const std::string& texturePath,
           const std::string& normalMapPath);

  void SetScale(float scale = 1.f);
  void SetCreateInfo(glm::vec3 center, glm::vec3 scale, glm::vec2 uvScale);

  void Load(vks::VertexLayout layout,
            vks::VulkanDevice* device,
            VkQueue copyQueue,
            bool useCreateInfo = false);

  std::string m_ModelPath;
  std::string m_TexturePath;
  std::string m_NormalMapPath;

  vks::Model m_Model;
  vks::Texture2D m_Texture;
  vks::Texture2D m_NormalMap;
  VkDescriptorSet m_DescriptorSet;
  vks::ModelCreateInfo m_ModelCreateInfo;

  std::shared_ptr<ModelMaterial> m_Material;

  std::shared_ptr<ModelMaterial> GetMaterial() { return m_Material; }

 protected:
  bool m_IsRendered = true;
};
