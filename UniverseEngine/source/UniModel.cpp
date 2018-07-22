#include "UniModel.h"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


UniModel::UniModel(std::string n) {
	m_Name = n;
	m_ModelCreateInfo.center = glm::vec3(0, 0, 0);
	m_ModelCreateInfo.scale = glm::vec3(1.f);
	m_ModelCreateInfo.uvscale = glm::vec2(1.f);
}

UniModel::UniModel(std::string n, const std::string &modelpath, const std::string &texturePath, const std::string &normalMapPath) {
	m_Name = n;
	m_ModelCreateInfo.center = glm::vec3(0, 0, 0);
	m_ModelCreateInfo.scale = glm::vec3(1.f);
	m_ModelCreateInfo.uvscale = glm::vec2(1.f);
	m_ModelPath = modelpath;
	m_TexturePath = texturePath;
	m_NormalMapPath = normalMapPath;
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

	// Textures
	std::string texFormatSuffix;
	VkFormat texFormat;
	// Get supported compressed texture format
	if(device->features.textureCompressionBC) {
		texFormatSuffix = "_bc3_unorm";
		texFormat = VK_FORMAT_BC3_UNORM_BLOCK;
	} else if(device->features.textureCompressionASTC_LDR) {
		texFormatSuffix = "_astc_8x8_unorm";
		texFormat = VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
	} else if(device->features.textureCompressionETC2) {
		texFormatSuffix = "_etc2_unorm";
		texFormat = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
	} else {
		vks::tools::exitFatal("Device does not support any compressed texture format!", VK_ERROR_FEATURE_NOT_PRESENT);
	}

	if(!m_TexturePath.empty()) {
		m_Texture.loadFromFile(getAssetPath() + m_TexturePath + texFormatSuffix + ".ktx", texFormat, device, copyQueue);
	} else {
		std::vector<glm::vec4> buffer(4 * 4);
		for(auto & i : buffer) {
			i = glm::vec4(.6f, .6f, .6f, 1.f);
		}
		m_Texture.fromBuffer(buffer.data(), buffer.size() * sizeof(glm::vec4), VK_FORMAT_R32G32B32A32_SFLOAT, 4, 4, device, copyQueue, VK_FILTER_LINEAR);
	}

	if(!m_NormalMapPath.empty()){
		m_NormalMap.loadFromFile(getAssetPath() + m_NormalMapPath + texFormatSuffix + ".ktx", texFormat, device, copyQueue);
	}
	else {
		std::vector<glm::vec4> buffer(4 * 4);
		for(auto & i : buffer) {
			i = glm::vec4(0.f, 0.f, 1.f, 0.f);
		}
		m_NormalMap.fromBuffer(buffer.data(), buffer.size() * sizeof(glm::vec4), VK_FORMAT_R32G32B32A32_SFLOAT, 4, 4, device, copyQueue, VK_FILTER_LINEAR);
	}


}

UniModel::~UniModel() {}
