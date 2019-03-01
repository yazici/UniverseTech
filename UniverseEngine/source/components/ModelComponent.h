#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include "../3dmaths.h"
#include "../UniModelMesh.h"
#include "../UniSceneObject.h"
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
                 std::vector<std::string> materials);
  void Destroy();
  ~ModelComponent();

  void SetScale(float scale = 1.f);
  void SetCreateInfo(glm::vec3 center, glm::vec3 scale, glm::vec2 uvScale);
  uint32_t GetMaterialIndex(std::string material);

  void Load(uni::VertexLayout layout,
            vks::VulkanDevice* device,
            VkQueue copyQueue,
            bool useCreateInfo = false);

  std::string m_ModelPath;
  std::string m_Name;

  uni::Model m_Model;
  uni::ModelCreateInfo m_ModelCreateInfo;

  std::string GetName() { return m_Name; }
  void SetSceneObject(std::shared_ptr<UniSceneObject> so) {
    m_SceneObject = so;
  }
  std::shared_ptr<UniSceneObject> GetSceneObject() { return m_SceneObject; }

  std::vector<std::string> GetMaterialIDs() { return m_Materials; }

 protected:
  bool m_IsRendered = true;
  std::shared_ptr<UniSceneObject> m_SceneObject;

 private:
  std::vector<std::string> m_Materials;
};
