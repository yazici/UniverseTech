#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include "3dmaths.h"
#include "UniSceneObject.h"
#include "vks/VulkanModel.hpp"
//#include "vks/VulkanTexture.hpp"
//#include "vulkan/vulkan_core.h"
//#include "materials/ModelMaterial.h" 

using json = nlohmann::json;

class UniModel : public UniSceneObject {
 public:
  UniModel() { m_Name = "unnamed model"; }
  UniModel(std::string n);
  UniModel(std::string n,
           const std::string& modelpath,
           const std::string& materialID);
  void Destroy();
  ~UniModel();

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
  vks::ModelCreateInfo m_ModelCreateInfo;

 protected:
  bool m_IsRendered = true;

 private:
  std::string m_MaterialID;
};
