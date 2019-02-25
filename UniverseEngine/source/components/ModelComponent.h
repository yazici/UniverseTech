#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include "../3dmaths.h"
#include "../vks/VulkanModel.hpp"
//#include "vks/VulkanTexture.hpp"
//#include "vulkan/vulkan_core.h"
//#include "materials/ModelMaterial.h" 

using json = nlohmann::json;

class ModelComponent {
 public:
  ModelComponent() { m_Name = "unnamed model"; }
  ModelComponent(std::string n);
  ModelComponent(std::string n,
           const std::string& modelpath,
           const std::string& materialID);
  void Destroy();
  ~ModelComponent();

  void SetScale(float scale = 1.f);
  void SetCreateInfo(glm::vec3 center, glm::vec3 scale, glm::vec2 uvScale);

  void Load(vks::VertexLayout layout,
            vks::VulkanDevice* device,
            VkQueue copyQueue,
            bool useCreateInfo = false);

  std::string m_ModelPath;
  std::string m_TexturePath;
  std::string m_NormalMapPath;
  std::string m_Name;

  vks::Model m_Model;
  vks::ModelCreateInfo m_ModelCreateInfo;

  std::string GetName() { return m_Name; }

 protected:
  bool m_IsRendered = true;

 private:
  std::string m_MaterialID;
};
