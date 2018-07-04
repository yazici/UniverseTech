#pragma once
#include <string>
#include "vks/VulkanModel.hpp"
#include "vks/VulkanTexture.hpp"
#include "vulkan/vulkan_core.h"
#include "UniSceneObject.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;


class UniModel : public UniSceneObject{
public:
	UniModel();
	~UniModel();

	UniModel(const std::string &modelpath, const std::string &texturePath, const std::string &normalMapPath);

	void SetScale(float scale = 1.f);
	void SetCreateInfo(glm::vec3 center, glm::vec3 scale, glm::vec2 uvScale);

	void Load(vks::VertexLayout layout, vks::VulkanDevice *device, VkQueue copyQueue, bool useCreateInfo = false);

	std::string m_ModelPath;
	std::string m_TexturePath;
	std::string m_NormalMapPath;

	vks::Model m_Model;
	vks::Texture2D m_Texture;
	vks::Texture2D m_NormalMap;
	VkDescriptorSet m_DescriptorSet;
	vks::ModelCreateInfo m_ModelCreateInfo;
};

