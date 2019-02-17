#include "UniModel.h"

UniModel::UniModel(std::string n) {
	m_Name = n;
	m_ModelCreateInfo.center = glm::vec3(0, 0, 0);
	m_ModelCreateInfo.scale = glm::vec3(1.f);
	m_ModelCreateInfo.uvscale = glm::vec2(1.f);
}

void UniModel::Destroy() {
  std::cout << "Destroying model..." << std::endl;

  m_Model.destroy();
}

UniModel::UniModel(std::string n, const std::string &modelpath, const std::string &materialID) {
	m_Name = n;
	m_ModelCreateInfo.center = glm::vec3(0, 0, 0);
	m_ModelCreateInfo.scale = glm::vec3(1.f);
	m_ModelCreateInfo.uvscale = glm::vec2(1.f);
	m_ModelPath = modelpath;
  m_MaterialID = materialID;
}

void UniModel::SetScale(float scale /*= 1.f*/) {
	m_ModelCreateInfo.scale = glm::vec3(scale);
}

void UniModel::SetCreateInfo(glm::vec3 center, glm::vec3 scale, glm::vec2 uvScale) {
	m_ModelCreateInfo.scale = scale;
	m_ModelCreateInfo.center = center;
	m_ModelCreateInfo.uvscale = uvScale;
}

void UniModel::Load(vks::VertexLayout layout, vks::VulkanDevice *device, VkQueue copyQueue, bool useCreateInfo) {
	if(useCreateInfo)
		m_Model.loadFromFile(getAssetPath() + m_ModelPath, layout, &m_ModelCreateInfo, device, copyQueue);
	else
		m_Model.loadFromFile(getAssetPath() + m_ModelPath, layout, 1.f, device, copyQueue);


}

UniModel::~UniModel() {}
