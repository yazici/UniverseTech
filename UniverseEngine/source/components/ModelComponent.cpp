#include "ModelComponent.h"
#include "../UniEngine.h"

ModelComponent::ModelComponent(std::string n) {
  m_Name = n;
  m_ModelCreateInfo.center = glm::vec3(0, 0, 0);
  m_ModelCreateInfo.scale = glm::vec3(1.f);
  m_ModelCreateInfo.uvscale = glm::vec2(1.f);
}

void ModelComponent::Destroy() {
  std::cout << "Destroying model..." << std::endl;

  m_Model.destroy();
}

ModelComponent::ModelComponent(std::string n,
                               const std::string& modelpath,
                               std::vector<std::string> materials) {
  m_Name = n;
  m_ModelCreateInfo.center = glm::vec3(0, 0, 0);
  m_ModelCreateInfo.scale = glm::vec3(1.f);
  m_ModelCreateInfo.uvscale = glm::vec2(1.f);
  m_ModelPath = modelpath;
  m_Materials = materials;
}

void ModelComponent::SetScale(float scale /*= 1.f*/) {
  m_ModelCreateInfo.scale = glm::vec3(scale);
}

void ModelComponent::SetCreateInfo(glm::vec3 center,
                                   glm::vec3 scale,
                                   glm::vec2 uvScale) {
  m_ModelCreateInfo.scale = scale;
  m_ModelCreateInfo.center = center;
  m_ModelCreateInfo.uvscale = uvScale;
}

uint32_t ModelComponent::GetMaterialIndex(std::string material) {
  uint32_t index = 0;
  for_each(m_Materials.begin(), m_Materials.end(), [&index, material](auto m) {
    if (m == material)
      return index;
    index++;
  });
  return index;
}

void ModelComponent::Load(uni::VertexLayout layout,
                          vks::VulkanDevice* device,
                          VkQueue copyQueue,
                          bool useCreateInfo) {

  auto engine = UniEngine::GetInstance();

  if (useCreateInfo)
    m_Model.loadFromFile(engine->getAssetPath() + m_ModelPath, layout,
                         &m_ModelCreateInfo, device, copyQueue, m_Materials);
    
  else
    m_Model.loadFromFile(engine->getAssetPath() + m_ModelPath, layout, 1.f, device,
                         copyQueue, m_Materials);
}

ModelComponent::~ModelComponent() {}
